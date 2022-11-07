// channel.h
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)


#ifndef ISA_CHANNEL_H
#define ISA_CHANNEL_H

#include <netdb.h>

union AnyIPAddress {
    sa_family_t family;
    struct sockaddr sock;
    struct sockaddr_in in;
    struct sockaddr_in6 in6;
};

struct RecvResult {
    ssize_t size; /**< The number of bytes read. */
    AnyIPAddress address /**< The received IP address. */;
    socklen_t address_len /**< The length of the received IP address. */;
    /**
     * True if the received IP address matches the destination address of the Channel that created this RecvResult.
     */
    bool is_channel_address;

    [[nodiscard]] bool success() const { return size != -1; }
};

struct Channel {
    int socket_fd; /**< File descriptor of the open RAW/ICMP socket. */
    union AnyIPAddress dst_addr; /**< Describes the destination IP address. */
    union AnyIPAddress src_addr; /**< Describes the source (interface) IP address. */
    socklen_t addr_len; /**< The length of the addresses in bytes. */

    /**
     * Opens a socket for communicating on the ICMP(v6) over IPv4 or IPv6, based on the family specified in
     * the dst address.
     *
     * @param [in] dst A pointer to a sockaddr that contains the destination IP address.
     * @param [in] src A pointer to a sockaddr that contains the source (interface) IP address.
     * @param [in] if_name The name of the source interface.
     * @param [in] receive_timeout_ms The receive timeout in milliseconds. If zero, receive timeout is not set.
     * @return A Channel with the opened socked file descriptor and both parties
     */
    Channel(sockaddr *dst, sockaddr *src, const std::string &if_name, unsigned int receive_timeout_ms = 0);

    /**
     * Creates a Channel that encapsulates an open socket.
     * @param [in] dst A pointer to a sockaddr that contains the destination IP address.
     * @param [in] src A pointer to a sockaddr that contains the source (interface) IP address.
     * @param socket_fd The socket file descriptor.
     */
    Channel(sockaddr *dst, sockaddr *src, int socket_fd);

    /**
     * Sets the receive timeout for the socket (modified the SO_RCVTIMEO option).
     * @param [in] timeout_ms The receive timeout in milliseconds.
     */
    void set_receive_timeout(unsigned int timeout_ms);

    /**
     * Waits until the socket is ready to send data, then sends data from an input buffer to the destination address
     * of this Channel.
     * @remark poll() is used to wait for the POLLOUT event.
     * @param [in] buf The input buffer.
     * @param [in] size Number of bytes to read from the input buffer and send.
     * @param [in] flags sendto() flags.
     * @return Returns the number of bytes sent, or -1 for errors.
     */
    ssize_t sendto_poll(const void *buf, size_t size, int flags) const;

    /**
     * Sends data from an input buffer to the destination address of this Channel.
     * @param [in] buf The input buffer.
     * @param [in] size Number of bytes to read from the input buffer and send.
     * @param [in] flags sendto() flags.
     * @return Returns the number of bytes sent; or -1 for errors.
     */
    ssize_t sendto(const void *buf, size_t size, int flags) const;

    /**
     * Receives data from the socket's receive buffer.
     * No address controls are performed.
     * @param [out] buf A buffer to store the data to.
     * @param [in] size Maximum number of bytes to store in @a buf.
     * @param [in] flags recvfrom() flags.
     * @param [out] recv_addr A reference to an AnyIPAddress structure to save the remote address to.
     * @param [in,out] recv_addr_len A reference to a socklen_t variable that signalizes the size of recv_addr.
     * This variable will be updated with the size of the address of the remote party.
     * @return Returns the number of bytes read; or -1 for errors.
     */
    ssize_t recvfrom(void *buf, size_t size, int flags, AnyIPAddress &recv_addr, socklen_t &recv_addr_len) const;

    /**
     * Receives data from the socket's receive buffer. Checks if the received address matches the destination
     * address of this channel.
     * @param [out] buf A buffer to store the data to.
     * @param [in] size Maximum number of bytes to store in @a buf.
     * @param [in] flags recvfrom() flags.
     * @return A RecvResult structure with number of bytes read, the received address and result of
     * the address comparison.
     */
    RecvResult recvfrom(void *buf, size_t size, int flags) const;

    /**
     * Closes the socket descriptor associated with this Channel.
     */
    ~Channel();

private:
    /**
     * Populates this instance's @a dst_addr and @a src_addr fields with a destination and a source address.
     * @param [in] dst The destination address.
     * @param [in] src The source address.
     */
    void fill_address_data(sockaddr *dst, sockaddr *src);
};

#endif //ISA_CHANNEL_H
