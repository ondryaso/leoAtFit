// channel.cpp
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)

#include <poll.h>
#include <unistd.h>

#include "errors.h"
#include "utils.h"
#include "channel.h"

using std::string;

Channel::Channel(sockaddr *dst, sockaddr *src, const string &if_name, unsigned int receive_timeout_ms)
        : dst_addr(), src_addr(), addr_len() {
    if (src == nullptr) {
        throw std::invalid_argument("src");
    }

    if (src->sa_family == AF_INET) {
        socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
        if (socket_fd == -1) {
            THROW_ERRNO();
        }

        // Binds to the specified interface name
        if (setsockopt(socket_fd, IPPROTO_IP, SO_BINDTODEVICE, if_name.c_str(), if_name.length()) == -1) {
            log_warn("setsockopt SO_BINDTODEVICE");
        }

        // Configures the socket to expect IP headers in the input buffer
        int on = 1;
        if (setsockopt(socket_fd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
            CLOSE_THROW_W(socket_fd, "setsockopt IP_HDRINCL");
        }

        // Set receive timeout
        if (receive_timeout_ms > 0) {
            set_receive_timeout(receive_timeout_ms);
        }
    } else if (src->sa_family == AF_INET6) {
        socket_fd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
        if (socket_fd == -1) {
            THROW_ERRNO();
        }

        // SO_BINDTODEVICE and IP_HDRINCL is not supported on AF_INET6

        // Set receive timeout
        if (receive_timeout_ms > 0) {
            set_receive_timeout(receive_timeout_ms);
        }
    }

    fill_address_data(dst, src);
}

Channel::Channel(sockaddr *dst, sockaddr *src, int socket_fd)
        : dst_addr(), src_addr(), addr_len() {
    this->socket_fd = socket_fd;

    if (src->sa_family == AF_INET) {
        // Configures the socket to expect IP headers in the input buffer
        int on = 1;
        if (setsockopt(socket_fd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
            CLOSE_THROW_W(socket_fd, "setsockopt IP_HDRINCL");
        }
    }

    fill_address_data(dst, src);
}

void Channel::set_receive_timeout(unsigned int timeout_ms) {
    struct timeval tv{
            .tv_sec = (timeout_ms / 1000),
            .tv_usec = ((timeout_ms % 1000) * 1000)
    };

    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1) {
        CLOSE_THROW_W(socket_fd, "setsockopt SO_RCVTIMEO");
    }
}

Channel::~Channel() {
    if (socket_fd >= 0) {
        close(socket_fd);
        socket_fd = -1;
    }
}

ssize_t Channel::sendto(const void *buf, size_t size, int flags) const {
    return ::sendto(socket_fd, buf, size, flags, &dst_addr.sock, addr_len);
}

ssize_t Channel::sendto_poll(const void *buf, size_t size, int flags) const {
    pollfd poll_opts[1] = {{.fd = socket_fd, .events = POLLOUT, .revents = 0}};
    if (poll(poll_opts, 1, -1) == -1) {
        THROW_ERRNO();
    }

    return ::sendto(socket_fd, buf, size, flags, &dst_addr.sock, addr_len);
}

ssize_t Channel::recvfrom(void *buf, size_t size, int flags, AnyIPAddress &recv_addr, socklen_t &recv_addr_len) const {
    return ::recvfrom(socket_fd, buf, size, flags, &recv_addr.sock, &recv_addr_len);
}

RecvResult Channel::recvfrom(void *buf, size_t size, int flags) const {
    RecvResult result{};
    result.address_len = sizeof(result.address);
    result.size = ::recvfrom(socket_fd, buf, size, flags, &result.address.sock, &result.address_len);
    if (result.size != -1) {
        result.is_channel_address = sockaddr_eq(&dst_addr.sock, &result.address.sock);
    }

    return result;
}

void Channel::fill_address_data(sockaddr *dst, sockaddr *src) {
    if (src->sa_family == AF_INET) {
        in_addr src_addr_p = reinterpret_cast<sockaddr_in *>(src)->sin_addr;
        src_addr.in = {.sin_family = AF_INET, .sin_addr = src_addr_p};

        if (dst != nullptr) {
            in_addr dst_addr_p = reinterpret_cast<sockaddr_in *>(dst)->sin_addr;
            dst_addr.in = {.sin_family = AF_INET, .sin_addr = dst_addr_p};
        } else {
            dst_addr = src_addr;
        }

        addr_len = sizeof(sockaddr_in);
    } else if (src->sa_family == AF_INET6) {
        in6_addr src_addr_p = reinterpret_cast<sockaddr_in6 *>(src)->sin6_addr;
        src_addr.in6 = {.sin6_family = AF_INET6, .sin6_addr = src_addr_p};

        if (dst != nullptr) {
            in6_addr dst_addr_p = reinterpret_cast<sockaddr_in6 *>(dst)->sin6_addr;
            dst_addr.in6 = {.sin6_family = AF_INET6, .sin6_addr = dst_addr_p};
        } else {
            dst_addr = src_addr;
        }

        addr_len = sizeof(sockaddr_in6);
    }
}
