// interface_find.cpp
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)

#include "errors.h"
#include "utils.h"
#include "common.h"
#include "packet_utils.h"
#include "interface_find.h"

using std::string;

bool InterfaceFinder::find_source_interface(addrinfo *target, string &if_name, sockaddr *&src_address,
                                            sockaddr *&dst_address) {
    load_interface_list();

    while (target != nullptr) {
        log_verbose("Finding source interface for target address "
                            << addr_to_string(target->ai_addr));

        if (target->ai_family == AF_INET) {
            for (const auto &item: ipv4Ifaces) {
                if (send_ping(target->ai_addr, item.first, item.second)) {
                    if_name = item.second;
                    src_address = item.first;
                    dst_address = target->ai_addr;
                    return true;
                }
            }
        } else if (target->ai_family == AF_INET6) {
            for (const auto &item: ipv6Ifaces) {
                if (send_ping(target->ai_addr, item.first, item.second)) {
                    if_name = item.second;
                    src_address = item.first;
                    dst_address = target->ai_addr;
                    return true;
                }
            }
        }

        target = target->ai_next;
    }

    return false;
}

void InterfaceFinder::load_interface_list() {
    log_verbose("Loading interface list");

    if (addresses != nullptr) {
        freeifaddrs(addresses);
    }

    ipv4Ifaces.clear();
    ipv4Ifaces.clear();

    if (getifaddrs(&addresses) == -1) {
        addresses = nullptr;
        THROW_ERRNO();
    }

    struct ifaddrs *address = addresses;
    while (address) {
        if (address->ifa_addr == nullptr) {
            address = address->ifa_next;
            continue;
        }

        int family = address->ifa_addr->sa_family;

        if (family == AF_INET) {
            ipv4Ifaces.insert({address->ifa_addr, string(address->ifa_name)});
        } else if (family == AF_INET6) {
            ipv6Ifaces.insert({address->ifa_addr, string(address->ifa_name)});
        }

        address = address->ifa_next;
    }
}

InterfaceFinder::~InterfaceFinder() {
    if (addresses != nullptr) {
        freeifaddrs(addresses);
    }
}

bool InterfaceFinder::send_ping(sockaddr *dst, sockaddr *src, const string &if_name) {
    char data[6] = "Hello";

    size_t packet_len;
    if (dst->sa_family == AF_INET) {
        packet_len = IP4_HEADER_LEN + ICMP_HEADER_LEN + sizeof(data);
    } else {
        packet_len = IP6_HEADER_LEN + ICMP_HEADER_LEN + sizeof(data);
    }

    uint8_t packet_buffer[packet_len];
    uint8_t *packet;
    memset(packet_buffer, 0, packet_len);

    auto id = getpid();
    uint16_t total_len = make_icmp_packet(packet_buffer, packet, dst, src, id, 0, data, sizeof(data));

    Channel channel = Channel(dst, src, if_name, INTERFACE_FIND_RECV_TIMEOUT_MS);
    int socket_fd = channel.socket_fd;

    // Send 'ping'
    if (sendto(socket_fd, packet, total_len, 0, &channel.dst_addr.sock, channel.addr_len) == -1) {
        close(socket_fd);
        return false;
    }

    // Receive response
    AnyIPAddress recv_addr{};
    socklen_t recv_addr_len = channel.addr_len;

    while (!sockaddr_eq(&recv_addr.sock, dst)) {
        auto received = recvfrom(socket_fd, packet_buffer, packet_len, 0, &recv_addr.sock, &recv_addr_len);

        if (received == -1) {
            close(socket_fd);
            return false;
        }
    }

    close(socket_fd);
    return true;
}
