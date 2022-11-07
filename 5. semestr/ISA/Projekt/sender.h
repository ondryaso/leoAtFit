// sender.h
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)

#ifndef ISA_SENDER_H
#define ISA_SENDER_H

#include <string>
#include <istream>
#include <limits>
#include <vector>

#include "common.h"
#include "channel.h"
#include "secure_string.h"


class Sender {
public:
    /**
     * Creates a Sender instance. Resolves the specified hostname (which may be an IP address). Finds the source
     * interface for this address. Discovers the maximum size of packet (MPS) that can be sent over the network to
     * the address. Initializes buffers and opens a raw socket for communication.
     *
     * @param [in] target_hostname The remote party hostname or IP address.
     * @throws std::runtime_error Thrown when a fatal error occurs when establishing the communication parameters.
     */
    explicit Sender(const std::string &target_hostname);

    /**
     * Transfers the contents of an input stream to a file named @a file_name on the receiver side.
     *
     * @param [in,out] stream The input stream to read transferred data from.
     * @param [in] file_name The file name to send to the receiver.
     * @param [in] password The encryption password (a shared secret to generate the data encryption key from).
     * @throws std::runtime_error Thrown when a fatal error occurs during the transmission.
     */
    void send(std::istream &stream, const std::string &file_name, const secure_string &password);

    /**
     * Closes the communication Channel and frees the memory allocated for the buffers.
     */
    ~Sender();

private:
    TransmissionMode mode; /**< The current mode of transmission. */
    Channel *channel; /**< Encapsulates the socket currently used for communication. */
    std::string target_interface;  /**< The name of the interface to communicate on. */
    uint8_t *packet_buffer; /**< A buffer for preparing packets to send. */
    char *data_buffer; /**< A buffer for preparing data to send. */
    uint16_t packet_len; /**< Length of packet_buffer. Dynamically decided based on the established MPS. */
    uint16_t data_len; /**< Length of data_buffer. Dynamically decided based on the established MPS. */
    uint16_t trans_id; /**< The current transaction ID. */
    uint16_t max_seq = 0; /**< The largest chunk sequence number that has been sent. */

    /**
     * Performs a protocol handshake: Sends a 'Hello' packet and waits for MODE_ESTAB_TIMEOUT_MS milliseconds to
     * receive a 'Hello Back' packet from the receiver. If it does, sets the transmission mode to TWO_WAY and
     * sends a 'Hello Back' packet to the receiver.
     */
    void com_init();

    /**
     * Sends a 'File Information' packet to the receiver with the specified file name (encrypted) and file length
     * information. In two-way mode, waits for a confirmation packet. If it arrives and signalises an error, throws.
     *
     * @param [in] file_name The file name to announce to the receiver.
     * @param [in] file_len The file length to announce to the receiver.
     * @param [in] password The encryption password (a shared secret to generate the data encryption key from).
     */
    void com_send_fileinfo(const std::string &file_name, std::streamsize file_len, const secure_string &password);

    /**
     * Reads the input stream, encrypts it and sends it to the receiver. When done, sends an 'All Data Sent'
     * protocol packet (seq = UINT16_MAX).
     *
     * @remark Set S_WAIT_FOR_DATA_REPLIES to 1 to wait for an Echo Reply message to confirm delivery of each sent chunk.
     * This is purely experimental and does not cause the packet to be resent.
     * @param [in] stream The input stream.
     * @param [in] password The encryption password (a shared secret to generate the data encryption key from).
     * @param seq_whitelist If provided, only sends chunks the sequence number of which is present in the vector. This
     * is used when resending missed chunks.
     */
    void com_send_data(std::istream &stream, const secure_string &password,
                       std::vector<uint16_t> *seq_whitelist = nullptr);

    /**
     * Waits for a protocol message from the receiver. If it requests resending some chunks, seeks stream to the
     * beginning and calls com_send_data() to resend the specified chunks.
     *
     * @remark This is only used in two-way mode.
     * @param [in] stream The originally sent data input stream.
     * @param [in] password The encryption password (a shared secret to generate the data encryption key from).
     */
    void com_receive_result(std::istream &stream, const secure_string &password);

    /**
     * Determines the number of bytes that can be read from the stream.
     * Requires the stream to be seekable.
     *
     * @param [in,out] stream The input stream.
     * @return The number of bytes that can be read from the stream.
     */
    static std::streamsize get_file_size(std::istream &stream);
};

#endif //ISA_SENDER_H
