// receiver.cpp
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)


#include <fcntl.h>
#include <algorithm>
#include <sys/mman.h>
#include <poll.h>
#include <filesystem>

#include "receiver.h"
#include "encryption.h"
#include "utils.h"
#include "interface_find.h"
#include "errors.h"
#include "packet_utils.h"

void Receiver::accept(const secure_string &password) {
    log_info("Waiting for a transmission");

    int in_socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (in_socket_fd == -1) {
        THROW_ERRNO();
    }

    int in6_socket_fd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (in6_socket_fd == -1) {
        CLOSE_THROW(in_socket_fd);
    }

    int socket_fds[2]{in_socket_fd, in6_socket_fd};
    pollfd poll_opts[2] = {{.fd = in_socket_fd, .events = POLLIN, .revents = 0},
                           {.fd = in6_socket_fd, .events = POLLIN, .revents = 0}};

    while (true) {
        int poll_res = poll(poll_opts, 2, -1);
        if (poll_res == -1) {
            auto errno_prev = errno;
            close(in_socket_fd);
            close(in6_socket_fd);

            if (errno_prev == EINTR) {
                log_verbose("Signal received");
                return;
            } else {
                throw std::system_error(errno_prev, std::generic_category(), "poll()");
            }
        }

        log_verbose("poll() out");
        for (int i = 0; i < 2; i++) {
            if (poll_opts[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                close(in_socket_fd);
                close(in6_socket_fd);
                throw std::runtime_error("poll() failed");
            } else if (poll_opts[i].revents & POLLIN) {
                sa_family_t family = i == 0 ? AF_INET : AF_INET6;

                if (check_socket(socket_fds[i], family)) {
                    // We found our protocol's hello message, close the other socket
                    close(socket_fds[(i + 1) % 2]);

                    com_init();
                    com_receive_fileinfo(password);
                    com_receive_data();
                    decrypt_file(password);

                    return;
                }
            }
        }
    }
}

bool Receiver::check_socket(int socket_fd, sa_family_t family) {
    AnyIPAddress recv_addr{};
    socklen_t recv_addr_len = sizeof(recv_addr);

    log_verbose("Receiving from socket #" << socket_fd);
    ssize_t recv_len = recvfrom(socket_fd, input_buffer, R_RECV_BUFFER_SIZE, 0, &recv_addr.sock, &recv_addr_len);
    if (recv_len == -1) {
        CLOSE_THROW(socket_fd);
    }

    ICMPEchoHeader recv_header{};
    uint8_t *data_begin;
    uint16_t data_len;

    if (!parse_icmp_echo_packet(reinterpret_cast<uint8_t *>(input_buffer), recv_len, family, recv_header,
                                data_begin, data_len)) {
        // It's okay that this ICMP packet is not an Echo one
        return false;
    }

    // Correct hello packet: filled transmission ID, seq=0, data=0x01100110
    if (recv_header.seq != 0 || data_len != 4) {
        return false;
    }

    char hello[] = PROTO_HELLO;
    if (memcmp(hello, data_begin, 4) != 0) {
        return false;
    }

    log_info("Incoming transmission detected (" << addr_to_string(&recv_addr.sock) << ")");
    trans_id = recv_header.id;

    if (family == AF_INET) {
        // With IPv4, we can determine the source IP from the headers in the received packet
        ip *ip_header = reinterpret_cast<ip *>(input_buffer);
        sockaddr_in src_addr{.sin_family = AF_INET, .sin_addr = ip_header->ip_dst};
        channel = new Channel(&recv_addr.sock, reinterpret_cast<sockaddr *>(&src_addr),
                              socket_fd);
    } else {
        // With IPv6, we cannot determine the source IP
        // We don't need it though, so let's just fill it with a placeholder value
        sockaddr_in6 src_addr{.sin6_family = AF_INET6, .sin6_addr = {}};
        channel = new Channel(&recv_addr.sock, reinterpret_cast<sockaddr *>(&src_addr),
                              socket_fd);
    }

    return true;
}

void Receiver::com_init() {
    log_verbose("Attempting handshake");

    // Try sending 'Hello Back' packet
    char hello_resp[4] = PROTO_HELLO_BACK;

    uint8_t *packet;
    uint16_t packet_size;

    RecvResult received{};
    ICMPEchoHeader recv_header{};
    uint8_t *recv_data_begin;
    uint16_t recv_data_len;

    // Sending our Hello packet
    packet_size = make_icmp_packet(reinterpret_cast<uint8_t *>(input_buffer), packet, channel, trans_id, 0,
                                   hello_resp, 4);
    if (channel->sendto(packet, packet_size, 0) == -1) {
        throw std::runtime_error("Cannot send hello back packet");
    }
    log_monitor("ESTAB: Sent R->S 'Hello Back'");

    // Waiting for ICMP packets to arrive
    channel->set_receive_timeout(MODE_ESTAB_TIMEOUT_MS);

    while (true) {
        log_monitor("ESTAB: Receiving data");

        received = channel->recvfrom(input_buffer, R_RECV_BUFFER_SIZE, 0);
        if (!received.success()) {
            // Timeout or error, assume one-way
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                log_monitor("ESTAB: Timeout");
            } else {
                THROW_ERRNO();
            }

            log_info("Set mode to one-way");
            mode = ONE_WAY;
            return;
        }

        if (!received.is_channel_address) {
            // Not an answer from the other side, read another one
            log_monitor("ESTAB: Data not from the expected address (actual: "
                                << addr_to_string(&received.address.sock) << ")");
            continue;
        }

        if (!parse_icmp_echo_packet(reinterpret_cast<uint8_t *>(input_buffer), received.size,
                                    channel->dst_addr.family, recv_header, recv_data_begin, recv_data_len)) {
            // Invalid ICMP Echo packet (either malformed or non-echo)
            log_monitor("ESTAB: Data not a valid ICMP Echo packet");
            continue;
        }

        if (recv_header.id != trans_id) {
            log_monitor("ESTAB: Echo message doesn't belong to us (id "
                                << static_cast<int>(recv_header.id) << ")");
            continue;
        }

        if (recv_header.type != ICMP_ECHO && recv_header.type != ICMP6_ECHO_REQUEST) {
            log_monitor("ESTAB: Got Echo Response message, ignoring");
            continue;
        }

        if (memcmp(recv_data_begin, hello_resp, 4) == 0) {
            // We have received an echo request, we can use the two-way mode
            log_monitor("ESTAB: Received 'Hello Back' Echo Request from the other side, set mode to two-way");
            mode = TWO_WAY;
            return;
        }
    }
}

void Receiver::com_receive_fileinfo(const secure_string &password) {
    log_verbose("Receiving file info");
    channel->set_receive_timeout(R_FILE_INFO_TIMEOUT_MS);

    RecvResult received;
    ICMPEchoHeader recv_header{};
    uint8_t *recv_data_begin;
    uint16_t recv_data_len;

    while (true) {
        received = channel->recvfrom(input_buffer, R_RECV_BUFFER_SIZE, 0);
        if (!received.success()) {
            // Timeout or error, assume one-way
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                log_monitor("INFO: Timeout");
                throw std::runtime_error("File information did not arrive in time");
            } else {
                THROW_ERRNO();
            }
        }

        if (!received.is_channel_address) {
            // Not an answer from the other side, read another one
            log_monitor("INFO: Data not from the expected address (actual: "
                                << addr_to_string(&received.address.sock) << ")");
            continue;
        }

        if (!parse_icmp_echo_packet(reinterpret_cast<uint8_t *>(input_buffer), received.size,
                                    channel->dst_addr.family, recv_header, recv_data_begin, recv_data_len)) {
            // Invalid ICMP Echo packet (either malformed or non-echo)
            log_monitor("INFO: Data not a valid ICMP Echo packet");
            continue;
        }

        if (recv_header.id != trans_id) {
            log_monitor("INFO: Echo message doesn't belong to us (id "
                                << static_cast<int>(recv_header.id) << ")");
            continue;
        }

        if (recv_header.seq != 1) {
            log_monitor("INFO: Seq not one");
            continue;
        }

        if (recv_header.type != ICMP_ECHO && recv_header.type != ICMP6_ECHO_REQUEST) {
            log_monitor("INFO: Got Echo Response message, ignoring");
            continue;
        }

        break;
    }

    unsigned char decrypt_buf[recv_data_len + 1];
    auto crypto = Crypto(password, trans_id);

    size_t decrypt_len;
    try {
        decrypt_len = crypto.decrypt(decrypt_buf, recv_data_begin, recv_data_len, true);
    } catch (std::runtime_error const &e) {
        com_send_control(PROTO_INVALID_KEY, "");
        throw e;
    }

    if (decrypt_len <= sizeof(uint64_t)) {
        throw std::runtime_error("Invalid file information received");
    }

    // Add null termination character to the end of decrypted data so that std::string may easily be created from it
    decrypt_buf[decrypt_len] = 0;
    recv_encrypted_file_len = be64toh(*reinterpret_cast<uint64_t *>(decrypt_buf));
    std::string file_name = std::string(reinterpret_cast<char *>(decrypt_buf + sizeof(uint64_t)));

    log_info("Accepting file: " << file_name << " (" << recv_encrypted_file_len << " bytes long)");
    open_file(file_name);
    open_tmp_file();
}

void Receiver::open_file(const string &name) {
    // Check if the file exists
    auto file_path = std::filesystem::path(name);
    if (std::filesystem::exists(file_path) && !enable_overwrite) {
        com_send_control(PROTO_ERROR, "File already exists");
        throw std::runtime_error("An existing file '" + name + "' would be overwritten");
    }

    // Attempt to allocate the file and map it to memory
    int fd = open(file_path.c_str(), O_RDWR | O_CREAT, 600);
    if (fd == -1) {
        log_warn("Cannot open file: " << strerror(errno));
    } else {
        out_file_fd = fd;

        int allocate_res = posix_fallocate(fd, 0, static_cast<long>(recv_encrypted_file_len));
        if (allocate_res == 0) {
            out_file_mem = mmap(nullptr, recv_encrypted_file_len, PROT_WRITE, MAP_SHARED, fd, 0);
            if (out_file_mem == MAP_FAILED) {
                log_warn("Cannot memory-map file: " << strerror(errno));
                close(fd);
                out_file_mem = nullptr;
            }
        } else {
            log_warn("Cannot allocate file space");
            close(fd);
        }
    }

    // If it failed, attempt to open the file 'normally'
    if (out_file_mem == nullptr) {
        out_file = fopen(file_path.c_str(), "wb");

        if (out_file == nullptr) {
            auto errno_prev = errno;
            com_send_control(PROTO_ERROR, "Cannot open file for writing");
            throw std::system_error(errno_prev, std::generic_category(),
                                    "Cannot open file " + file_path.string() + " for writing");
        }
    }
}

void Receiver::open_tmp_file() {
#if R_USE_TEMP_FILE == 1
    tmp_file = std::tmpfile();
    if (tmp_file == nullptr) {
        throw std::runtime_error("Cannot open a temporary file for writing the received data");
    }

    int fd = fileno(tmp_file);
    int allocate_res = posix_fallocate(fd, 0, static_cast<long>(recv_encrypted_file_len));
    if (allocate_res == 0) {
        tmp_file_mem = mmap(nullptr, recv_encrypted_file_len, PROT_WRITE, MAP_SHARED, fd, 0);
        if (tmp_file_mem == MAP_FAILED) {
            log_warn("Cannot memory-map temporary file: " << strerror(errno));
            tmp_file_mem = nullptr;
        }
    } else {
        log_warn("Cannot allocate temporary file space");
    }
#else
    tmp_file_mem = malloc(recv_encrypted_file_len);
    if (tmp_file_mem == nullptr) {
        throw std::runtime_error("Cannot allocate memory for the received file");
    }
#endif
}

void Receiver::decrypt_file(const secure_string &password) {
    log_verbose("Decrypting file");
    auto crypto = Crypto(password, trans_id + 1);

    if (tmp_file_mem != nullptr) {
        auto encrypted = reinterpret_cast<const unsigned char *>(tmp_file_mem);

        if (out_file_mem != nullptr) {
            // Easiest - "memory to memory"
            auto decrypted = crypto.decrypt(reinterpret_cast<unsigned char *>(out_file_mem),
                                            encrypted, recv_encrypted_file_len, true);

            log_verbose("Decrypted " << decrypted << " bytes");
            munmap(out_file_mem, recv_encrypted_file_len);
            if (ftruncate(out_file_fd, static_cast<long>(decrypted)) == -1) {
                perror("Cannot truncate output file");
            }
        } else {
            // Temp file is memory-mapped, output file is not
            unsigned char crypto_buf[R_RECV_BUFFER_SIZE];
            auto input_len = Crypto::max_plain_len_to_fit(R_RECV_BUFFER_SIZE);
            size_t input_pos = 0;

            while (input_pos < recv_encrypted_file_len) {
                auto decrypted_len = crypto.decrypt(crypto_buf, encrypted + input_pos, input_len,
                                                    input_pos + input_len >= recv_encrypted_file_len);
                fwrite(crypto_buf, 1, decrypted_len, out_file);
                input_pos += input_len;
            }
        }

#if R_USE_TEMP_FILE == 1
        munmap(tmp_file_mem, recv_encrypted_file_len);
#else
        free(tmp_file_mem);
#endif
        tmp_file_mem = nullptr;
    } else {
        // Temp file is not memory-mapped
        if (out_file_mem != nullptr) {
            unsigned char input_buf[R_RECV_BUFFER_SIZE];
            size_t read;
            size_t output_pos = 0;
            size_t input_pos = 0;

            while ((read = fread(input_buf, 1, R_RECV_BUFFER_SIZE, tmp_file)) > 0) {
                auto decrypted_len = crypto.decrypt(reinterpret_cast<unsigned char *>(out_file_mem) + output_pos,
                                                    input_buf, read,
                                                    input_pos + R_RECV_BUFFER_SIZE >= recv_encrypted_file_len);
                input_pos += read;
                output_pos += decrypted_len;
            }

            munmap(out_file_mem, recv_encrypted_file_len);
            if (ftruncate(out_file_fd, static_cast<long>(output_pos)) == -1) {
                perror("Cannot truncate output file");
            }
        } else {
            // Both the temp file and the output file are opened as files and not mapped to memory
            unsigned char input_buf[R_RECV_BUFFER_SIZE];
            unsigned char output_buf[R_RECV_BUFFER_SIZE];

            size_t read;
            size_t input_pos = 0;

            while ((read = fread(input_buf, 1, R_RECV_BUFFER_SIZE, tmp_file)) > 0) {
                auto decrypted_len = crypto.decrypt(output_buf, input_buf, read,
                                                    input_pos + R_RECV_BUFFER_SIZE >= recv_encrypted_file_len);
                input_pos += read;
                fwrite(output_buf, 1, decrypted_len, out_file);
            }
        }

        fclose(tmp_file);
        tmp_file = nullptr;
    }
}

void Receiver::com_receive_data() {
    uint16_t sender_last_seq = 0;
    bool had_last_seq = last_seq_received;

    uint16_t last_seq = 1;

    RecvResult received;
    ICMPEchoHeader recv_header{};
    uint8_t *recv_data_begin;
    uint16_t recv_data_len;

    uint16_t expected_trans_id = trans_id + 1;

    uint64_t written = 0;
    uint64_t last_end_pos = 0;

    channel->set_receive_timeout(R_RECV_TIMEOUT_MS);
    while (true) {
        received = channel->recvfrom(input_buffer, R_RECV_BUFFER_SIZE, 0);
        if (!received.success()) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                THROW_ERRNO();
            }
        }

        if (!received.is_channel_address) {
            // Not an answer from the other side, read another one
            log_monitor("DATA: Data not from the expected address (actual: "
                                << addr_to_string(&received.address.sock) << ")");
            continue;
        }

        if (!parse_icmp_echo_packet(reinterpret_cast<uint8_t *>(input_buffer), received.size,
                                    channel->dst_addr.family, recv_header, recv_data_begin, recv_data_len)) {
            // Invalid ICMP Echo packet (either malformed or non-echo)
            log_monitor("DATA: Data not a valid ICMP Echo packet");
            continue;
        }

        if (recv_header.id != expected_trans_id) {
            log_monitor("DATA: Echo message doesn't belong to us (id "
                                << static_cast<int>(recv_header.id) << ")");
            continue;
        }

        if (recv_header.type != ICMP_ECHO && recv_header.type != ICMP6_ECHO_REQUEST) {
            continue;
        }

        if (recv_header.seq < 2) {
            log_monitor("DATA: Got seq < 2, ignoring");
            continue;
        }

        if (recv_header.seq == UINT16_MAX) {
            // 'All Data Sent' packet received
            if (recv_data_len < sizeof(uint16_t)) {
                log_warn("Invalid ending packet received; terminating");
                return;
            }

            log_verbose("Ending packet received");
            sender_last_seq = be16toh(*reinterpret_cast<uint16_t *>(recv_data_begin));
            last_seq_received = true;

            continue;
        }

        uint64_t pos = *reinterpret_cast<uint64_t *>(recv_data_begin);
        pos = be64toh(pos);
        uint32_t encrypted_data_len = recv_data_len - sizeof(uint64_t);

        if ((pos + encrypted_data_len) > recv_encrypted_file_len) {
            log_warn("total len: " << recv_encrypted_file_len << ", current pos: " << pos << ", recv len: "
            << encrypted_data_len);
            // Don't allow writing outside the expected bounds
            throw std::runtime_error(
                    "Invalid write position requested. This may signalise an attack attempt, terminating");
        }

        if (tmp_file_mem != nullptr) {
            memcpy((char *) tmp_file_mem + pos, recv_data_begin + sizeof(uint64_t), encrypted_data_len);
        } else {
            if (last_end_pos != pos) {
                fseek(tmp_file, static_cast<long>(pos), SEEK_SET);
            }

            fwrite(recv_data_begin + sizeof(uint64_t), 1, encrypted_data_len, tmp_file);
            last_end_pos = pos + encrypted_data_len;
        }

        written += encrypted_data_len;
        log_monitor("DATA [" << recv_header.seq << "]: Written " << encrypted_data_len << " B to pos " << pos);

        uint16_t &seq = recv_header.seq;
        if (first_pass) {
            missed_seqs.remove(seq);

            // Keep track of missed seqs
            if (seq != (last_seq + 1)) {
                log_verbose("Previous seq: " << last_seq << ", current: " << seq);
                if (seq > last_seq) {
                    for (uint16_t i = last_seq + 1; i < seq; i++) {
                        missed_seqs.push_back(i);
                    }
                } else {
                }
            }

            if (seq > last_seq) {
                last_seq = seq;
            }
            if (seq > highest_seq_received) {
                highest_seq_received = seq;
            }
        } else {
            missed_seqs.remove(seq);
        }
    }

    if (first_pass) {
        if (last_seq < sender_last_seq) {
            for (uint16_t i = last_seq + 1; i <= sender_last_seq; i++) {
                missed_seqs.push_back(i);
            }
        } else if (last_seq > sender_last_seq) {
            if (sender_last_seq == 0) {
                log_warn("Ending packet not received, cannot guarantee completeness");
            } else {
                log_warn("More chunks received than expected");
            }
        }
    } else if (had_last_seq != last_seq_received) {
        for (uint16_t i = highest_seq_received + 1; i <= sender_last_seq; i++) {
            missed_seqs.push_back(i);
        }
    }

    // Clear receive queue
    RecvResult r;
    channel->set_receive_timeout(1);
    do {
        r = channel->recvfrom(input_buffer, R_RECV_BUFFER_SIZE, 0);
    } while (r.success() && r.size > 0);

    // In one-way mode, we can only inform the sender of success/failure
    // In two-way mode, we can check whether we have received all data and ask for more or signalise success
    if (mode == ONE_WAY) {
#if ENABLE_MONITOR == 1
        for (const auto &item: missed_seqs) {
            log_monitor("DATA: Packet with seq " << item << " missing");
        }
#endif

        if (!missed_seqs.empty()) {
            log_warn("Some data were not received, data WILL be CORRUPTED");
            return;
        }
    } else {
        if (missed_seqs.empty()) {
            log_verbose("All data received");
            // We have received everything
            com_send_control(PROTO_OK, "");
        } else {
            log_verbose("Missing " << missed_seqs.size() << " seqs, requesting resending");
            // Request resend
            size_t err_data_len = sizeof(uint8_t) + missed_seqs.size() * sizeof(uint16_t);
            char err_data[err_data_len];
            err_data[0] = PROTO_REQUEST_RESEND;
            size_t err_data_pos = 1;

            int i = 0;
            for (const auto &item: missed_seqs) {
                // 730 entries will fill one 1500 B IP packet and leave a bit of space just for sure
                if (++i > 730) {
                    break;
                }

                uint16_t item_be = htobe16(item);
                memcpy(err_data + err_data_pos, &item_be, sizeof(uint16_t));
                err_data_pos += sizeof(uint16_t);
            }

            uint8_t *packet;
            auto packet_size = make_icmp_packet(reinterpret_cast<uint8_t *>(input_buffer), packet, channel, trans_id,
                                                UINT16_MAX,
                                                err_data, err_data_pos);

            if (channel->sendto(packet, packet_size, 0) == -1) {
                throw std::runtime_error("Cannot send 'Request Resend' packet");
            }

            // Start listening for incoming packets again
            log_verbose("Listening for incoming packets again");
            first_pass = false;
            com_receive_data();
        }
    }
}

void Receiver::com_send_control(uint8_t flag, const string &message) {
    char err_data[sizeof(uint8_t) + message.length()];
    if (message.length() > 0) {
        memcpy(err_data + sizeof(uint8_t), message.c_str(), message.length());
    }

    err_data[0] = static_cast<char>(flag);

    uint8_t *packet;
    auto packet_size = make_icmp_packet(reinterpret_cast<uint8_t *>(input_buffer), packet, channel, trans_id,
                                        UINT16_MAX,
                                        err_data, sizeof(err_data));

    if (channel->sendto(packet, packet_size, 0) == -1) {
        throw std::runtime_error("Cannot send control packet");
    }

    log_monitor("Sent control packet (type " << static_cast<int>(flag) << ", message: '" << message << "')");
}

Receiver::~Receiver() {
    if (out_file != nullptr) {
        fclose(out_file);
        out_file = nullptr;
    }

    if (tmp_file_mem != nullptr) {
#if R_USE_TEMP_FILE == 1
        munmap(tmp_file_mem, recv_encrypted_file_len);
#else
        free(tmp_file_mem);
#endif
        tmp_file_mem = nullptr;
    }
#if R_USE_TEMP_FILE == 1
    else if (tmp_file != nullptr) {
        fclose(tmp_file);
    }
#endif
}

