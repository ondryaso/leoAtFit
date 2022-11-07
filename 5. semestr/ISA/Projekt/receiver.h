// receiver.h
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)


#ifndef ISA_RECEIVER_H
#define ISA_RECEIVER_H

#include <string>
#include <fstream>
#include <list>

#include "common.h"
#include "secure_string.h"
#include "channel.h"

class Receiver {
public:
    explicit Receiver(bool enable_overwrite) : mode(ONE_WAY), enable_overwrite(enable_overwrite) {};

    /**
     * Opens two raw sockets – one for IPv4, one for IPv6 – and listens on both in parallel until a 'Hello' protocol
     * is received. Then closes the other socket and performs all steps of data transmission.
     *
     * @param [in] password The encryption password (a shared secret to generate the data encryption key from).
     */
    void accept(const secure_string &password);

    ~Receiver();

private:
    TransmissionMode mode; /**< The current mode of transmission. */
    Channel *channel{}; /**< Encapsulates the socket currently used for communication. */
    char input_buffer[R_RECV_BUFFER_SIZE]{}; /**< A buffer for storing received data. */
    uint16_t trans_id{}; /**< The current transaction ID. */

    bool enable_overwrite; /**< Signalises if Receiver should allow overwriting an existing file. */
    uint64_t recv_encrypted_file_len{}; /**< The total length of the encrypted data that is being received. */
    std::list<uint16_t> missed_seqs; /**< A list of sequence numbers of chunks that haven't been received. */

    /** Signalises if a 'Transmission Finished' protocol packet with total number of chunks has been received. */
    bool last_seq_received = false;
    uint16_t highest_seq_received = 0; /**< The largest chunk sequence number that has been received. */

    /**
     * FILE identifier of the temporary file used to store received encrypted data.
     * Populated if R_USE_TEMP_FILE is set to 1.
     */
    FILE *tmp_file{};
    /**
     * Pointer to memory-mapped temporary file used to store received encrypted data.
     * Populated if R_USE_TEMP_FILE is set to 1.
     * */
    void *tmp_file_mem{};

    FILE *out_file{}; /**< FILE identifier of the output file to save decrypted data to. */
    int out_file_fd; /**< File descriptor number of the output file to save decrypted data to. */
    void *out_file_mem{}; /**< Pointer to memory-mapped output file to save decrypted data to. */

    /**< Signalises if the Receiver is receiving the initial stream of data (not data it has requested to resend). */
    bool first_pass = true;

    /**
     * Receives a packet from a socket. Checks if it contains a valid 'Hello' message.
     * If so, populates channel and trans_id.
     *
     * @param [in] socket_fd The socket file descriptor.
     * @param [in] family The address family of the socket (AF_INET or AF_INET6).
     * @return True if the received packet contains a valid 'Hello' message; false otherwise.
     */
    bool check_socket(int socket_fd, sa_family_t family);

    /**
     * Opens the specified file as the output file. Attempts to pre-allocate space of @a recv_encrypted_file_len bytes
     * and to map the file into memory. Populates out_file, out_file_fd and out_file_mem.
     *
     * @remark out_file is only opened if memory mapping fails.
     * @param [in] name Name of the file.
     * @throws std::runtime_error Thrown when the file cannot be opened.
     */
    void open_file(const std::string &name);

    /**
     * If R_USE_TEMP_FILE is set to 1, creates a temporary file, stores its identifier to tmp_file and attempts
     * to pre-allocate space of @a recv_encrypted_file_len bytes and to map the file into memory.
     * If R_USE_TEMP_FILE is set to 0, allocates @a recv_encrypted_file_len bytes in memory.
     *
     * @throws std::runtime_error Thrown when the file cannot be opened (or when memory cannot be allocated).
     */
    void open_tmp_file();

    /**
     * Reads the received encrypted data from the temporary file or memory, decrypts them and stores them
     * to the output file. Attempts to truncate the file to the resulting number of decrypted bytes.
     *
     * @param [in] password The encryption password (a shared secret to generate the data encryption key from).
     * @throws std::runtime_error Thrown when decryption fails.
     */
    void decrypt_file(const secure_string &password);

    /**
     * Finalizes a received handshake: Sends a 'Hello Back' packet to the remote party and waits for a confirmation
     * 'Hello Back' packet to be received from the sender. If this succeeds, sets the transmission mode to two-way.
     */
    void com_init();

    /**
     * Receives a 'File Information' packet, decrypts the received file name, stores the received file length in
     * @a recv_encrypted_file_en and calls open_file() and open_tmp_file().
     *
     * @param [in] password The encryption password (a shared secret to generate the data encryption key from).
     * @throws std::runtime_error Thrown when the packet doesn't arrive in time specified by R_FILE_INFO_TIMEOUT_MS.
     * @throws std::runtime_error Thrown when the received packet doesn't contain valid data.
     * @throws std::runtime_error Thrown when decryption fails.
     * @see R_FILE_INFO_TIMEOUT_MS
     */
    void com_receive_fileinfo(const secure_string &password);

    /**
     * Receives the data stream. Saves the received data to the temporary file/memory.
     * Keeps track of sequence number of chunks that were not received.
     * If the transmission runs in two-way mode, sends back a 'Request Resend' packet for the first 730 missed chunks
     * and calls itself again.
     */
    void com_receive_data();

    /**
     * Sends a protocol message – an ICMP Echo Request with one byte of data, optionally followed by a string message.
     *
     * @param [in] flag The protocol message type.
     * @param [in] message The message. May be empty.
     */
    void com_send_control(uint8_t flag, const std::string &message);
};

#endif //ISA_RECEIVER_H
