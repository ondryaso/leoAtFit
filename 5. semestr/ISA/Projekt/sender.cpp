// sender.h
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)

#include <sys/socket.h>
#include <vector>
#include <algorithm>

#include "encryption.h"
#include "utils.h"
#include "interface_find.h"
#include "sender.h"
#include "packet_utils.h"

using std::istream;
using std::string;

Sender::Sender(const string &target_hostname) : mode(ONE_WAY) {
    // Resolve destination
    auto target_addr = resolve(target_hostname);

    // Find source IP address
    InterfaceFinder interface_finder;
    sockaddr *src_addr, *dst_addr;
    bool iface_found = interface_finder.find_source_interface(target_addr, target_interface, src_addr, dst_addr);
    if (!iface_found) {
        throw std::runtime_error("Destination unreachable");
    }

    log_verbose("Source IP: " << addr_to_string(src_addr));

    // Find maximum packet size
    auto mps_channel = Channel(dst_addr, src_addr, target_interface, MPS_RECV_TIMEOUT_MS);
    auto mps = find_max_packet_size(mps_channel, STARTING_MPS);

    packet_buffer = new uint8_t[mps.packet_size];
    data_buffer = new char[mps.data_size];
    packet_len = mps.packet_size;
    data_len = mps.data_size;

    srand(getpid());
    trans_id = rand() % UINT16_MAX;

    // Create channel
    channel = new Channel(dst_addr, src_addr, target_interface);
}

void Sender::send(istream &stream, const string &file_name, const secure_string &password) {
    if (stream.eof()) {
        return;
    }

    // Throw when the stream gets into an erroneous state
    stream.exceptions(std::iostream::failbit | std::iostream::badbit);

    // If the input stream doesn't support seeking, this will throw
    auto stream_size = get_file_size(stream);

    com_init();
    com_send_fileinfo(file_name, stream_size, password);
    com_send_data(stream, password);
}

void Sender::com_init() {
    // Send hello (0x01 10 01 10)
    char hello[] = PROTO_HELLO;
    char hello_resp[] = PROTO_HELLO_BACK;

    uint8_t *packet;
    uint16_t packet_size;

    RecvResult received;
    ICMPEchoHeader recv_header{};
    uint8_t *recv_data_begin;
    uint16_t recv_data_len;

    uint state = 0;

    com_init_fsm:
    switch (state) {
        case 0: // Sending our Hello packet
            packet_size = make_icmp_packet(packet_buffer, packet, channel, trans_id, 0, hello, 4);
            if (channel->sendto_poll(packet, packet_size, 0) == -1) {
                throw std::runtime_error("Cannot send 'Hello' packet");
            }
            log_monitor("ESTAB: Sent first S->R 'Hello'");

            state = 1;
            goto com_init_fsm;
        case 1: // Waiting for ICMP packets to arrive
            channel->set_receive_timeout(MODE_ESTAB_TIMEOUT_MS);
            log_monitor("ESTAB: Receiving data");

            received = channel->recvfrom(packet_buffer, packet_len, 0);
            if (!received.success()) {
                // Timeout or error, assume one-way
                log_monitor("ESTAB: Timeout or error");
                state = 99;
                goto com_init_fsm;
            }

            if (!received.is_channel_address) {
                // Not an answer from the other side, read another one
                log_monitor("ESTAB: Data not from the expected address (actual: "
                                    << addr_to_string(&received.address.sock) << ")");
                state = 1;
                goto com_init_fsm;
            }

            if (!parse_icmp_echo_packet(packet_buffer, received.size, channel->dst_addr.family, recv_header,
                                        recv_data_begin, recv_data_len)) {
                // Invalid ICMP Echo packet (either malformed or non-echo)
                log_monitor("ESTAB: Data not a valid ICMP Echo packet");
                state = 1;
                goto com_init_fsm;
            }

            if (recv_header.id != trans_id) {
                log_monitor("ESTAB: Echo message doesn't belong to us (id "
                                    << static_cast<int>(recv_header.id) << ")");
                state = 1;
                goto com_init_fsm;
            }

            if (recv_header.type != ICMP_ECHO && recv_header.type != ICMP6_ECHO_REQUEST) {
                log_monitor("ESTAB: Got Echo Response message, ignoring");
                state = 1;
                goto com_init_fsm;
            }

            state = 2;
            goto com_init_fsm;
        case 2:
            if (memcmp(recv_data_begin, hello, 4) == 0) {
                // We have received an echo response
                log_monitor("ESTAB: Received a 'Hello' packet, ignoring");
                state = 1;
                goto com_init_fsm;
            }

            if (memcmp(recv_data_begin, hello_resp, 4) == 0) {
                // We have received an echo request, we can use the two-way mode
                log_monitor("ESTAB: Received a 'Hello Back' packet from the other side");
                state = 3;
                goto com_init_fsm;
            }

            // We haven't received anything useful, let's wait one more time
            state = 1;
            goto com_init_fsm;
        case 3:
            log_monitor("ESTAB: Sending two-way confirmation 'Hello Back' Echo Request");
            packet_size = make_icmp_packet(packet_buffer, packet, channel, trans_id, 0, hello_resp, 4);
            if (channel->sendto_poll(packet, packet_size, 0) == -1) {
                throw std::runtime_error("Cannot send 'Hello Back' packet");
            }

            log_info("Set mode to two-way");
            mode = TWO_WAY;
            return;
        case 99:
            log_info("Set mode to one-way");
            mode = ONE_WAY;
            return;
    }
}


void Sender::com_send_fileinfo(const string &file_name, std::streamsize file_len, const secure_string &password) {
    log_info("Sending file information");

    // Check if we have enough space to send the data
    auto len = sizeof(uint64_t) + file_name.size();
    auto req_packet_len = Crypto::encrypted_len(len);
    if (req_packet_len > data_len) {
        throw std::runtime_error("Cannot transmit file information: packet too large");
    }

    // Encryption input buffer
    unsigned char input_buf[len];

    // Put encrypted data size (big endian) to the buffer
    uint64_t file_size = htobe64(Crypto::encrypted_len(file_len));
    memcpy(input_buf, &file_size, sizeof(uint64_t));

    // Put file name to the buffer
    memcpy(input_buf + sizeof(uint64_t), file_name.c_str(), file_name.size());

    // Encrypt data
    auto crypto = Crypto(password, trans_id);
    auto encrypted_len =
            crypto.encrypt(reinterpret_cast<unsigned char *>(data_buffer), input_buf, len, true);

    // Make packet
    uint8_t *res_packet;
    uint16_t packet_size = make_icmp_packet(packet_buffer, res_packet, channel, trans_id, 1, data_buffer,
                                            encrypted_len);

    // Send packet
    if (channel->sendto_poll(res_packet, packet_size, 0) == -1) {
        throw std::runtime_error("Cannot send file information packet");
    }

    log_monitor("INFO: Sent file information (encrypted data length: " << encrypted_len << ")");

    // TODO: Two-way mode (wait for response)
    // in two-way mode, signalise failure with an exception
}

void Sender::com_send_data(istream &stream, const secure_string &password, std::vector<uint16_t> *seq_whitelist) {
    log_info("Sending data");
    // Determine maximum chunk size
    auto max_chunk_size = Crypto::max_plain_len_to_fit(data_len - sizeof(uint64_t));

    // Create crypto for encryption (let salt be PID + 1)
    auto crypto = Crypto(password, trans_id + 1);

    unsigned char input_buf[max_chunk_size];
    uint16_t seq = 2;
    uint64_t pos = 0;

    // We've previously set the stream to throw when failbit is set
    // We don't want that now
    stream.exceptions(std::iostream::badbit);

    std::vector<uint16_t>::iterator it;
    if (seq_whitelist != nullptr) {
        std::sort(seq_whitelist->begin(), seq_whitelist->end());
        it = seq_whitelist->begin();
    }

#if S_WAIT_FOR_DATA_REPLIES == 1
    RecvResult received;
    ICMPEchoHeader recv_header{};
    uint8_t *recv_data_begin;
    uint16_t recv_data_len;
#endif

    while (!stream.eof()) {
        // Read from stream
        stream.read(reinterpret_cast<char *>(input_buf), static_cast<std::streamsize>(max_chunk_size));
        auto read = stream.gcount();
        // Determine whether we've read the last chunk
        auto is_last = stream.eof();
        // Encrypt data
        auto encrypted_len = crypto.encrypt(reinterpret_cast<unsigned char *>(data_buffer + sizeof(uint64_t)),
                                            input_buf, read, is_last);

        // If we're resending data, check whether we actually want to do it
        if (seq_whitelist != nullptr) {
            if (it == seq_whitelist->end()) {
                break;
            }

            if (*it != seq) {
                seq++;
                pos += encrypted_len;
                continue;
            }
            it++;
        }

        // Put encrypted data position into the packet
        uint64_t pos_be = htobe64(pos);
        memcpy(data_buffer, &pos_be, sizeof(uint64_t));
        pos += encrypted_len;
        // Make packet
        uint8_t *res_packet;
        uint16_t packet_size = make_icmp_packet(packet_buffer, res_packet, channel, trans_id + 1, seq++, data_buffer,
                                                encrypted_len + sizeof(uint64_t));

        // Send packet
        if (channel->sendto_poll(res_packet, packet_size, 0) == -1) {
            throw std::runtime_error("Cannot send file data chunk");
        }

        log_monitor("DATA: Sent chunk #" << (seq - 2) << " (encrypted data length: " << encrypted_len << ")");

        if (seq_whitelist == nullptr) {
#if S_DELAY_AFTER_SENT_CHUNKS > 0
            if (seq % S_DELAY_AFTER_SENT_CHUNKS == 0) {
                usleep(S_DELAY_US);
            }
#endif
        } else {
#if S_RESEND_DELAY_AFTER_SENT_CHUNKS > 0
            if (seq % S_RESEND_DELAY_AFTER_SENT_CHUNKS == 0) {
                usleep(S_RESEND_DELAY_US);
            }
#endif
        }


#if S_WAIT_FOR_DATA_REPLIES == 1
        while (true) {
            received = channel->recvfrom(packet_buffer, packet_len, 0);
            if (!received.success()) {
                break;
            }
            if (!received.is_channel_address) {
                continue;
            }
            if (!parse_icmp_echo_packet(packet_buffer, received.size, channel->dst_addr.family, recv_header,
                                        recv_data_begin, recv_data_len)) {
                continue;
            }
            if (recv_header.id != trans_id || (recv_header.type != ICMP_ECHOREPLY && recv_header.type != ICMP6_ECHO_REPLY)) {
                continue;
            }
            if (recv_header.seq != seq - 1) {
                // It would be good to keep track of received non-matching seqs here so that we don't drop anything
                continue;
            }
            break;
        }
        log_monitor("DATA: Confirmed chunk #" << (seq - 2));
#endif
    }

#if S_WAIT_FOR_DATA_REPLIES == 0
    // Clear receive queue
    RecvResult r;
    channel->set_receive_timeout(100);
    do {
        r = channel->recvfrom(packet_buffer, packet_len, 0);
    } while (r.success() && r.size > 0);
#endif

    if (max_seq == 0) {
        max_seq = seq - 1;
    }

    // Make end packet (seq = UINT16 maximum, data: last transferred seq)
    uint16_t last_seq = htobe16(max_seq);
    memcpy(data_buffer, &last_seq, sizeof(uint16_t));

    uint8_t *res_packet;
    uint16_t packet_size = make_icmp_packet(packet_buffer, res_packet, channel, trans_id + 1, UINT16_MAX,
                                            data_buffer,
                                            sizeof(uint16_t));
    usleep(S_CONFIRMATION_DELAY_US);
    // Send packet
    log_verbose("Sending confirmation packet");
    if (channel->sendto_poll(res_packet, packet_size, 0) == -1) {
        throw std::runtime_error("Cannot send file data chunk");
    }


    if (mode == TWO_WAY) {
        com_receive_result(stream, password);
    } else {
        log_info("File sent");
    }
}

void Sender::com_receive_result(std::istream &stream, const secure_string &password) {
    RecvResult received;
    ICMPEchoHeader recv_header{};
    uint8_t *recv_data_begin;
    uint16_t recv_data_len;

    log_verbose("Waiting for result");
    channel->set_receive_timeout(S_RECV_TIMEOUT_MS);
    while (true) {
        received = channel->recvfrom(packet_buffer, R_RECV_BUFFER_SIZE, 0);
        if (!received.success()) {
            // Timeout or error, assume one-way
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                log_info("No status confirmation received, file may not be transferred completely");
                return;
            } else {
                THROW_ERRNO();
            }
        }

        if (!received.is_channel_address) {
            // Not an answer from the other side, read another one
            log_monitor("END: Data not from the expected address (actual: "
                                << addr_to_string(&received.address.sock) << ")");
            continue;
        }

        if (!parse_icmp_echo_packet(reinterpret_cast<uint8_t *>(packet_buffer), received.size,
                                    channel->dst_addr.family, recv_header, recv_data_begin, recv_data_len)) {
            // Invalid ICMP Echo packet (either malformed or non-echo)
            log_monitor("END: Data not a valid ICMP Echo packet");
            continue;
        }

        if (recv_header.id != trans_id) {
            continue;
        }

        if (recv_header.seq != UINT16_MAX) {
            log_monitor("END: Invalid seq");
            continue;
        }

        if (recv_header.type != ICMP_ECHO && recv_header.type != ICMP6_ECHO_REQUEST) {
            log_monitor("END: Got Echo Response message, ignoring");
            continue;
        }

        break;
    }

    if (recv_data_begin[0] == PROTO_OK) {
        log_info("File sent successfully");
        return;
    } else if (recv_data_begin[0] == PROTO_REQUEST_RESEND) {
        log_verbose("Resend requested");
        if (((recv_data_len - sizeof(uint8_t)) % sizeof(uint16_t) != 0)) {
            log_warn("Invalid protocol message received");
            return;
        }

        std::vector<uint16_t> missing_seqs;
        for (size_t pos = sizeof(uint8_t); pos < recv_data_len; pos += sizeof(uint16_t)) {
            uint16_t seq = *reinterpret_cast<uint16_t *>(recv_data_begin + pos);
            seq = be16toh(seq);
            missing_seqs.push_back(seq);
        }

        if (!missing_seqs.empty()) {
            log_verbose("Resending data");
            stream.clear();
            stream.seekg(0, std::ios_base::beg);
            com_send_data(stream, password, &missing_seqs);
        }
    } else {
        log_warn("Unknown protocol message received");
        return;
    }
}

std::streamsize Sender::get_file_size(istream &stream) {
    stream.ignore(std::numeric_limits<std::streamsize>::max());
    auto len = stream.gcount();
    stream.clear();
    stream.seekg(0, std::ios_base::beg);
    return len;
}

Sender::~Sender() {
    delete channel;
    delete packet_buffer;
    delete data_buffer;
}
