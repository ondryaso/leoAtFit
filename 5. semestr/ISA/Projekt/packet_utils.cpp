// packet_utils.cpp
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)


#include "packet_utils.h"

using std::string;

struct addrinfo *resolve(const string &hostname) {
    log_info("Resolving hostname " << hostname);

    struct addrinfo hints{
            .ai_flags = AI_ADDRCONFIG,
            .ai_family = AF_UNSPEC,
            .ai_socktype = SOCK_STREAM,
            .ai_protocol = 0
    };

    struct addrinfo *res;
    int addr_info_res;
    if ((addr_info_res = getaddrinfo(hostname.c_str(), nullptr, &hints, &res)) != 0) {
        throw std::runtime_error("Cannot resolve '" + hostname + "': " + string(gai_strerror(addr_info_res)));
    }

    return res;
}

/*
 * Author: P. D. Buchan (pdbuchan@yahoo.com)
 * Source: https://www.pdbuchan.com/rawsock/icmp4.c
 * Distributed under the GNU GPL.
 */
uint16_t inet_checksum(uint16_t *value, int len) {
    int count = len;
    uint32_t sum = 0;
    uint16_t answer = 0;

    // Sum up 2-byte values until none or only one byte left.
    while (count > 1) {
        sum += *(value++);
        count -= 2;
    }

    // Add left-over byte, if any.
    if (count > 0) {
        sum += *(uint8_t *) value;
    }

    // Fold 32-bit sum into 16 bits; we lose information by doing this,
    // increasing the chances of a collision.
    // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    // Checksum is one's complement of sum.
    answer = ~sum;

    return (answer);
}

uint16_t make_icmp4_packet(uint8_t *res_packet, in_addr dst_addr, in_addr src_addr, uint16_t id, uint16_t seq,
                           char *data, uint16_t len) {
    struct ip ip_header{};
    struct icmp icmp_header{};

    // Create IPv4 header
    ip_header.ip_v = 4; // Version
    ip_header.ip_hl = IP4_HEADER_LEN / 4; // IHL (length in 32 bit words -> divide by 4 bytes)
    ip_header.ip_tos = 0; // Type of Service
    ip_header.ip_len = htons(IP4_HEADER_LEN + ICMP_HEADER_LEN + len); // Total Length
    ip_header.ip_id = htons(0); // Identification
    ip_header.ip_off = 0; // Flags + Fragment Offset (all zero = May Fragment, Last Fragment, Fragment Offset 0)
    ip_header.ip_ttl = 255; // Time to Live
    ip_header.ip_p = IPPROTO_ICMP; // Protocol (ICMP)
    ip_header.ip_src = src_addr;
    ip_header.ip_dst = dst_addr;
    ip_header.ip_sum = inet_checksum(reinterpret_cast<uint16_t *>(&ip_header), IP4_HEADER_LEN);

    // Create ICMP header (without checksum)
    icmp_header.icmp_type = ICMP_ECHO; // ICMP Type 8 (echo message)
    icmp_header.icmp_code = 0; // Code 0 (per RFC792)
    icmp_header.icmp_id = htons(id); // Identifier (arbitrary number)
    icmp_header.icmp_seq = htons(seq); // Sequence Number (arbitrary number)

    // Prepare packet
    memcpy(res_packet, &ip_header, IP4_HEADER_LEN);
    memcpy(res_packet + IP4_HEADER_LEN, &icmp_header, ICMP_HEADER_LEN);
    memcpy(res_packet + IP4_HEADER_LEN + ICMP_HEADER_LEN, data, len);

    // Calculate ICMP checksum
    uint16_t checksum = inet_checksum(reinterpret_cast<uint16_t *>(res_packet + IP4_HEADER_LEN),
                                      ICMP_HEADER_LEN + len);

    // Insert checksum into the packet
    auto icmp_checksum_p = reinterpret_cast<uint16_t *>(res_packet + IP4_HEADER_LEN + 2);
    *icmp_checksum_p = checksum;

    return IP4_HEADER_LEN + ICMP_HEADER_LEN + len;
}

uint16_t make_icmp6_packet(uint8_t *res_buffer, uint8_t *&res_packet, in6_addr dst_addr, in6_addr src_addr,
                           uint16_t id, uint16_t seq, char *data, uint16_t data_len) {
    struct icmp6_hdr icmp_header{};

    uint16_t payload_len = ICMP_HEADER_LEN + data_len;

    // Create ICMPv6 header
    icmp_header.icmp6_type = ICMP6_ECHO_REQUEST; // ICMP Type 128
    icmp_header.icmp6_code = 0; // Code 0 (per RFC4443
    icmp_header.icmp6_dataun.icmp6_un_data16[0] = htons(id); // Identifier
    icmp_header.icmp6_dataun.icmp6_un_data16[1] = htons(seq); // Sequence number

    // Insert ICMPv6 payload into the packet
    memcpy(res_buffer + IP6_HEADER_LEN, &icmp_header, ICMP_HEADER_LEN);
    memcpy(res_buffer + IP6_HEADER_LEN + ICMP_HEADER_LEN, data, data_len);

    // Calculate ICMPv6 checksum
    // ICMPv6 requires prepending an IPv6 pseudo-header
    memcpy(res_buffer, &src_addr, 16);
    memcpy(res_buffer + 16, &dst_addr, 16);
    *reinterpret_cast<uint32_t *>(res_buffer + 32) = htonl(payload_len);
    *reinterpret_cast<uint32_t *>(res_buffer + 36) = htonl(IPPROTO_ICMPV6); // Next Header pseudo value (RFC4443, 2.3)

    uint16_t checksum = inet_checksum(reinterpret_cast<uint16_t *>(res_buffer),
                                      IP6_HEADER_LEN + ICMP_HEADER_LEN + data_len);

    // Insert checksum into the packet
    *reinterpret_cast<uint16_t *>(res_buffer + IP6_HEADER_LEN + 2) = checksum;

    // The data to send over the socket starts in the middle of the packet buffer
    res_packet = res_buffer + IP6_HEADER_LEN;

    return ICMP_HEADER_LEN + data_len;
}

uint16_t make_icmp_packet(uint8_t *res_buffer, uint8_t *&res_packet, sockaddr *dst, sockaddr *src, uint16_t id,
                          uint16_t seq, char *data, uint16_t data_len) {
    if (dst->sa_family == AF_INET) {
        in_addr dst_addr = reinterpret_cast<sockaddr_in *>(dst)->sin_addr;
        in_addr src_addr = reinterpret_cast<sockaddr_in *>(src)->sin_addr;

        res_packet = res_buffer;
        return make_icmp4_packet(res_buffer, dst_addr, src_addr, id, seq, data, data_len);
    } else if (dst->sa_family == AF_INET6) {
        in6_addr dst_addr = reinterpret_cast<sockaddr_in6 *>(dst)->sin6_addr;
        in6_addr src_addr = reinterpret_cast<sockaddr_in6 *>(src)->sin6_addr;

        return make_icmp6_packet(res_buffer, res_packet, dst_addr, src_addr, id, seq, data, data_len);
    }

    return -1;
}

uint16_t make_icmp_packet(uint8_t *res_buffer, uint8_t *&res_packet, Channel *channel, uint16_t id, uint16_t seq,
                          char *data, uint16_t data_len) {
    return make_icmp_packet(res_buffer, res_packet, &channel->dst_addr.sock, &channel->src_addr.sock, id, seq,
                            data, data_len);
}

MPSResult find_max_packet_size(Channel &channel, uint16_t init_ceiling) {
    // Buffer for assembled packets
    uint8_t packet_buffer[init_ceiling];
    uint8_t *packet;

    // Header size (always the same)
    uint16_t header_size = (channel.dst_addr.family == AF_INET ? IP4_HEADER_LEN : IP6_HEADER_LEN)
                           + ICMP_HEADER_LEN;

    // Packet size and data size tried in the current step
    uint16_t current_packet_size = init_ceiling;
    uint16_t left_for_data = init_ceiling - header_size;

    // Binary search bounds
    uint16_t left = 0;
    uint16_t right = current_packet_size;

    // Binary search threshold
    const uint16_t threshold = MPS_SEARCH_THRESHOLD;

    // An array of zeros to use as the ICMP Echo Request data
    char dummy_data[left_for_data];
    memset(dummy_data, 0, left_for_data);

    AnyIPAddress recv_addr{};

    int socket_fd = channel.socket_fd;
    uint16_t seq = 0;
    uint64_t id = getpid();

    while (seq < init_ceiling) {
        uint16_t total_len = make_icmp_packet(packet_buffer, packet, &channel.dst_addr.sock, &channel.src_addr.sock,
                                              id, seq++, dummy_data, left_for_data);

        assert(total_len <= init_ceiling);
        log_verbose("Sending PSD packet #" << seq << " (trying size " << current_packet_size << ")");

        // Send ping
        if (sendto(socket_fd, packet, total_len, 0, &channel.dst_addr.sock, channel.addr_len) == -1) {
            if (errno == EMSGSIZE) {
                log_verbose("Packet not sent (EMSGSIZE)");
                // Kernel refused to send the message because it knows the MTU is lower
                // Try lower packet size
                right = current_packet_size;
                current_packet_size = left + ((right - left) / 2);
                left_for_data = current_packet_size - header_size;
            } else {
                THROW_ERRNO();
            }
        } else {
            // Wait for response
            socklen_t recv_addr_len = channel.addr_len;
            ssize_t received = 0;

            while (!sockaddr_eq(&channel.dst_addr.sock, &recv_addr.sock)) {
                log_verbose("Waiting for response for PSD packet #" << seq);
                received = recvfrom(socket_fd, packet_buffer, init_ceiling, 0, &recv_addr.sock, &recv_addr_len);

                if (received == -1) {
                    recv_addr = {};
                    break;
                }

                if (channel.dst_addr.family == AF_INET) {
                    auto recv_header = reinterpret_cast<icmp *>(packet + IP4_HEADER_LEN);
                    if (recv_header->icmp_type != ICMP_ECHOREPLY) {
                        recv_addr = {};
                        continue;
                    }
                } else {
                    auto recv_header = reinterpret_cast<icmp6_hdr *>(packet_buffer);
                    if (recv_header->icmp6_type != ICMP6_ECHO_REPLY) {
                        recv_addr = {};
                        continue;
                    }
                }
            }

            if (received == -1) {
                log_verbose("No response for PSD packet #" << seq << " (timeout)");

                // No response: consider the packet undelivered and try again
                right = current_packet_size;
                current_packet_size = left + ((right - left) / 2);

                if (current_packet_size <= header_size) {
                    throw std::runtime_error("cannot find maximum packet length (destination unreachable?)");
                }
                left_for_data = current_packet_size - header_size;
                continue;
            }

            log_verbose("Got response for PSD packet #" << seq);

            // We got a response, attempt increasing the value
            left = current_packet_size;
            size_t new_packet_size = left + ((right - left) / 2);

            if (new_packet_size >= init_ceiling || (new_packet_size - current_packet_size) <= threshold) {
                log_verbose("Target threshold " << threshold << " reached with maximum packet size "
                                                << current_packet_size);

                auto result = MPSResult{.packet_size = current_packet_size};
                result.data_size = left_for_data;
                return result;
            }

            current_packet_size = new_packet_size;
            left_for_data = current_packet_size - header_size;
        }
    }

    throw std::runtime_error("Cannot find maximum packet size");
}

bool parse_icmp_echo_packet(uint8_t *input_packet, uint16_t input_len, sa_family_t family, ICMPEchoHeader &header,
                            uint8_t *&data_begin, uint16_t &data_len) {
    static_assert(ICMP_HEADER_LEN == sizeof(ICMPEchoHeader));

    if (family == AF_INET) {
        ip *ip_header = reinterpret_cast<ip *>(input_packet);

        uint16_t total_len = ntohs(ip_header->ip_len);
        uint16_t header_len = ip_header->ip_hl * 4;

        if (ip_header->ip_v != 4 || ip_header->ip_p != IPPROTO_ICMP) {
            log_verbose("Invalid packet data for ICMP parsing");
            return false;
        }

        if (total_len < (header_len + ICMP_HEADER_LEN)) {
            return false;
        }

        if (input_len != total_len) {
            log_warn("IP datagram length mismatch: expected " << total_len << ", actual " << input_len);
        }

        if (input_packet[header_len] != ICMP_ECHO && input_packet[header_len] != ICMP_ECHOREPLY) {
            return false;
        }

        auto *icmp_header = reinterpret_cast<ICMPEchoHeader *>(input_packet + header_len);

        header = *icmp_header;
        header.id = ntohs(header.id);
        header.seq = ntohs(header.seq);

        data_begin = input_packet + header_len + ICMP_HEADER_LEN;
        data_len = total_len - header_len - ICMP_HEADER_LEN;
        return true;
    } else {
        // We don't get the IPv6 header so let's assume input_len contains the actual length of the received packet
        if (input_len < ICMP_HEADER_LEN) {
            return false;
        }

        if (input_packet[0] != ICMP6_ECHO_REQUEST && input_packet[0] != ICMP6_ECHO_REPLY) {
            return false;
        }

        // ICMP and ICMPv6 Echo Request/Reply messages have the same structure
        auto *icmp_header = reinterpret_cast<ICMPEchoHeader *>(input_packet);

        header = *icmp_header;
        header.id = ntohs(header.id);
        header.seq = ntohs(header.seq);

        data_begin = input_packet + ICMP_HEADER_LEN;
        data_len = input_len - ICMP_HEADER_LEN;
        return true;
    }
}
