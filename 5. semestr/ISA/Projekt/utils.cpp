// utils.cpp
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)

#include <cstring>
#include <arpa/inet.h>
#include <string>
#include <termios.h>
#include <unistd.h>
#include "utils.h"

#if ENABLE_MONITOR
uint64_t __monitor_counter = 1;
#endif

clock_t __start_time = clock();
bool enable_verbose = false, enable_info = true, enable_warn = true;

std::string addr_to_string(const sockaddr *address) {
    size_t len = address->sa_family == AF_INET ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN;
    char result[len];

    switch (address->sa_family) {
        case AF_INET:
            inet_ntop(AF_INET, &reinterpret_cast<const sockaddr_in *>(address)->sin_addr, result, len);
            break;
        case AF_INET6:
            inet_ntop(AF_INET6, &reinterpret_cast<const sockaddr_in6 *>(address)->sin6_addr, result, len);
            break;
        default:
            return "";
    }

    return result;
}

bool sockaddr_eq(const sockaddr *a, const sockaddr *b) {
    if (a->sa_family != b->sa_family)
        return false;

    if (a->sa_family == AF_INET) {
        return reinterpret_cast<const sockaddr_in *>(a)->sin_addr.s_addr ==
               reinterpret_cast<const sockaddr_in *>(b)->sin_addr.s_addr;
    } else if (a->sa_family == AF_INET6) {
        auto a_in6 = reinterpret_cast<const sockaddr_in6 *>(a);
        auto b_in6 = reinterpret_cast<const sockaddr_in6 *>(b);

        return memcmp(&a_in6->sin6_addr, &b_in6->sin6_addr, 16) == 0;
    }

    return false;
}

secure_string read_password() {
    termios termios_orig{};
    tcgetattr(STDIN_FILENO, &termios_orig);

    termios termios_mod = termios_orig;
    termios_mod.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &termios_mod);

    secure_string ret;

    int c;
    while (true) {
        c = getchar();
        if (c == 10 || c == EOF) {
            // Enter (line feed \n) or EOF
            break;
        }

        if (c == 127) {
            // Backspace
            ret.resize(ret.length() - 1);
        } else {
            ret += static_cast<char>(c);
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &termios_orig);
    return ret;
}

