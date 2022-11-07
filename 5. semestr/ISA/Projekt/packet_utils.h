// packet_utils.h
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)


#ifndef ISA_PACKET_UTILS_H
#define ISA_PACKET_UTILS_H

#include <string>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <iomanip>
#include <cassert>

#include "common.h"
#include "errors.h"
#include "utils.h"
#include "channel.h"

/**
 * A result of find_max_packet_size().
 */
struct MPSResult {
    uint16_t packet_size;
    uint16_t data_size;
};

struct ICMPEchoHeader {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t seq;
};

/**
 * Resolves the IPv4 and IPv6 addresses corresponding to the specified hostname.
 *
 * @param [in] hostname The hostname.
 * @throws runtime_error Thrown when the hostname cannot be resolved.
 * @return An addrinfo that represents a linked list of resolved IP addresses.
 */
struct addrinfo *resolve(const std::string &hostname);

/**
 * Computes an internet checksum.
 *
 * @param [in] addr A pointer to the start of data to calculate checksum for.
 * @param [in] len Total number of 32-bit words in the data.
 * @return The internet checksum for the specified data.
 * @see <a href="https://datatracker.ietf.org/doc/html/rfc1071">RFC 1071</a>
 */
uint16_t inet_checksum(uint16_t *addr, int len);

/**
 * Creates an IPv4 or IPv6 packet encapsulating an ICMP(v6) Echo Request with the specified data.
 * The IP version is decided based on the sa_family field of dst.
 *
 * @param [out] res_buffer A pointer to a buffer to save the packet data to.
 * @param [out] res_packet A reference to a pointer to the position in res_buffer where the actual packet data start.
 * @param [in] dst The destination IPv4 or IPv6 address.
 * @param [in] src The source (interface) IPv4 or IPv6 address.
 * @param [in] id A value for the ICMP Identification header field.
 * @param [in] seq A value for the ICMP Sequence Number header field.
 * @param [in] data A pointer to a buffer with the data to append to the ICMP Echo Request packet.
 * @param [in] data_len The length of the data.
 * @return Number of bytes written to the res_buffer buffer.
 */
uint16_t make_icmp_packet(uint8_t *res_buffer, uint8_t *&res_packet, sockaddr *dst, sockaddr *src, uint16_t id,
                          uint16_t seq, char *data, uint16_t data_len);

/**
 * Creates an IPv4 or IPv6 packet encapsulating an ICMP(v6) Echo Request with the specified data.
 * The IP version is decided based on the sa_family field of channel->dst_addr.
 *
 * @param [out] res_buffer A pointer to a buffer to save the packet data to.
 * @param [out] res_packet A reference to a pointer to the position in res_buffer where the actual packet data start.
 * @param [in] channel A Channel that contains the destination and source IPv4 or IPv6 addressed.
 * @param [in] id A value for the ICMP Identification header field.
 * @param [in] seq A value for the ICMP Sequence Number header field.
 * @param [in] data A pointer to a buffer with the data to append to the ICMP Echo Request packet.
 * @param [in] data_len The length of the data.
 * @return Number of bytes written to the res_packet buffer.
 */
uint16_t make_icmp_packet(uint8_t *res_buffer, uint8_t *&res_packet, Channel *channel, uint16_t id, uint16_t seq,
                          char *data, uint16_t data_len);

/**
 * Creates an IPv4 packet encapsulating an ICMP Echo Request with the specified data.
 *
 * @param [out] res_packet A pointer to a buffer to save the packet data to.
 * @param [in] dst_addr The destination IPv4 address.
 * @param [in] src_addr The source (interface) IPv4 address.
 * @param [in] id A value for the ICMP Identification header field.
 * @param [in] seq A value for the ICMP Sequence Number header field.
 * @param [in] data A pointer to a buffer with the data to append to the ICMP Echo Request packet.
 * @param [in] data_len The length of the data.
 * @return Number of bytes written to the res_packet buffer.
 */
uint16_t make_icmp4_packet(uint8_t *res_packet, in_addr dst_addr, in_addr src_addr, uint16_t id, uint16_t seq,
                           char *data, uint16_t data_len);

/**
 * Creates an IPv6 packet encapsulating an ICMPv6 Echo Request with the specified data.
 *
 * @param [out] res_buffer A pointer to a buffer to save the packet data to.
 * @param [out] res_packet A reference to a pointer to the position in res_buffer where the actual packet data start.
 * @param [in] dst_addr The destination IPv6 address.
 * @param [in] src_addr The source (interface) IPv6 address.
 * @param [in] id A value for the ICMP Identification header field.
 * @param [in] seq A value for the ICMP Sequence Number header field.
 * @param [in] data A pointer to a buffer with the data to append to the ICMP Echo Request packet.
 * @param [in] data_len The length of the data.
 * @return Number of bytes written to the res_buffer buffer.
 */
uint16_t make_icmp6_packet(uint8_t *res_buffer, uint8_t *&res_packet, in6_addr dst_addr, in6_addr src_addr,
                           uint16_t id, uint16_t seq, char *data, uint16_t data_len);

/**
 * Discovers the maximum size of an ICMP IP packet that can be delivered to the specified destination.
 *
 * @remarks
 * Uses modified binary search: it starts at the value of init_ceiling, if delivery is unsuccessful, the value
 * is halved until a sufficiently low value is found. This value is then used as the lower bound and from this moment,
 * a normal binary search ensues.
 * This behaviour is used because it is expected that init_ceiling has a fairly high chance of being the wanted value.
 * @remarks
 * MPS_SEARCH_THRESHOLD is used to determine the search accuracy.
 * @param [in] channel A descriptor of the communication channel.
 * @param [in] init_ceiling The initial maximum value of the IP packet size.
 * @return A MPSResult structure with the found maximum IP packet size and corresponding maximum data length.
 * @see MPS_SEARCH_THRESHOLD
 */
MPSResult find_max_packet_size(Channel &channel, uint16_t init_ceiling);

/**
 * Parses the data received from a socket as an ICMP Echo packet.
 * @param [in] input_packet An input buffer with the received data.
 * @param [in] input_len Length of the received data.
 * @param [in] family The family of the address (AF_INET or AF_INET6) this data came from.
 * @param [out] header A reference to an ICMPEchoHeader structure the parsed header will be saved to.
 * @param [out] data_begin A reference to a pointer to the place in the input buffer where the ICMP data part begins.
 * @param [out] data_len Length of the received ICMP data.
 * @return
 */
bool parse_icmp_echo_packet(uint8_t *input_packet, uint16_t input_len, sa_family_t family,
                            ICMPEchoHeader &header, uint8_t *&data_begin, uint16_t &data_len);

#endif //ISA_PACKET_UTILS_H
