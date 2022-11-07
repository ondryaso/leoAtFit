// utils.h
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)

#ifndef ISA_UTILS_H
#define ISA_UTILS_H

#include <ctime>
#include <netdb.h>
#include <limits>
#include <iostream>

#include "errors.h"
#include "secure_string.h"

// The length of an IPv4 header (with no options; in bytes)
#define IP4_HEADER_LEN 20
// The length of an ICMP header (in bytes)
#define ICMP_HEADER_LEN 8
// The length of an IPv6 header (in bytes)
#define IP6_HEADER_LEN 40

#define ENABLE_MONITOR 0
#if ENABLE_MONITOR
extern uint64_t __monitor_counter;
#define log_monitor(m) std::cout << "[VRB " << ((clock() - __start_time) /         \
    (CLOCKS_PER_SEC / 1000.0)) << "][" << __monitor_counter++ << "] " << m << ".\n"
#else
#define log_monitor(m)
#endif

extern clock_t __start_time;
extern bool enable_verbose, enable_info, enable_warn;
#define log_verbose(m) do { if (enable_verbose) { std::cout << "[VRB " << ((clock() - __start_time) / (CLOCKS_PER_SEC / 1000)) << "] " << m << '.' << std::endl; } } while(0)
#define log_info(m) do { if (enable_info) { std::cout << "[INF " << ((clock() - __start_time) / (CLOCKS_PER_SEC / 1000)) << "] " << m << '.' << std::endl; } } while(0)
#define log_warn(m) do { if (enable_warn) { std::cerr << "[WRN " << ((clock() - __start_time) / (CLOCKS_PER_SEC / 1000)) << "] " << m << '.' << std::endl; } } while(0)
#define log_err(m) std::cerr << "[ERR " << ((clock() - __start_time) / (CLOCKS_PER_SEC / 1000)) << "] " << m << ".\n" << std::flush

/**
 * Returns a string with the text representation of the specified IPv4 or IPv6 address.
 * @param address The IPv4 or IPv6 address.
 * @return A string with the text representation of the address.
 */
std::string addr_to_string(const sockaddr *address);

/**
 * Tests whether the specified IP addresses are equal.
 * @param a A pointer to sockaddr that contains an IPv4 or an IPv6 address.
 * @param b A pointer to sockaddr that contains an IPv4 or an IPv6 address.
 * @return True if the address in a equals the address in b.
 */
bool sockaddr_eq(const sockaddr *a, const sockaddr *b);

/**
 * Disables terminal echo, reads a password from the standard input and re-enables terminal echo.
 * @return The read password.
 */
secure_string read_password();

#endif //ISA_UTILS_H
