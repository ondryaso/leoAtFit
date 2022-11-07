// common.h
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)


#ifndef ISA_COMMON_H
#define ISA_COMMON_H

/**
 * Describes the used mode of transmission.
 */
enum TransmissionMode {
    /**
     * The sender doesn't expect transmission protocol messages from the receiver.
     * The sender is not reachable from the receiver side using ICMP Echo Request packets.
     */
    ONE_WAY,

    /**
     * The sender expects transmission protocol messages from the receiver (e.g. to confirm transmission start and end).
     * The sender is reachable from the receiver side using ICMP Echo Request packets.
     */
    TWO_WAY
};

/** Time to wait for a message from the receiver that signalizes that the two-way mode may be used. */
#define MODE_ESTAB_TIMEOUT_MS 1000

/** Time to wait for a ping reply when checking the interface to use for communication with a host. */
#define INTERFACE_FIND_RECV_TIMEOUT_MS 500

/**
 * Iteration count for the password-based key derivation routine,
 * RFC 2898 suggests an iteration count of at least 1000.
 * Source: https://www.openssl.org/docs/man1.1.0/man3/PKCS5_PBKDF2_HMAC_SHA1.html
 */
#define CRYPTO_KEY_DERIVATION_ITERATIONS 2048

/**
 * The initial (maximum) value for maximum packet size search.
 */
#define STARTING_MPS 1500

/**
 * Threshold for the maximum packet size binary search. When the difference between confirmed deliverable packet size
 * and the next attempt falls below this value, the confirmed size is considered the MPS.
 */
#define MPS_SEARCH_THRESHOLD 8

/** Time to wait for a ping reply when finding the maximum packet size. */
#define MPS_RECV_TIMEOUT_MS 500

/** Default encryption password (key) to use if the user doesn't specify a custom one. */
#define DEFAULT_PASSWORD "xondry02"

/* ------ Sender options ------ */

/** When set to 1, sender will wait for the Echo Reply message for each sent chunk. */
#define S_WAIT_FOR_DATA_REPLIES 0

/**
 * The sender will insert a delay of S_DELAY_US microseconds after each S_DELAY_AFTER_SENT_CHUNKS
 * sent chunks. Set to 0 to disable this behaviour. */
#define S_DELAY_AFTER_SENT_CHUNKS 16
#define S_DELAY_US 1000

/** Same as SENDER_DELAY_* but used when resending chunks. */
#define S_RESEND_DELAY_AFTER_SENT_CHUNKS 1
#define S_RESEND_DELAY_US 1000

/** Time in microseconds to wait before sending an 'All Data Sent' packet. */
#define S_CONFIRMATION_DELAY_US 100000

/** Time for the sender to wait for an incoming packet (confirmation protocol message). */
#define S_RECV_TIMEOUT_MS 10000

/* ------ Receiver options ------ */

/** Time for the receiver to wait for an incoming packet. */
#define R_RECV_TIMEOUT_MS 1000

/** Number of bytes to allocate for receiving data. */
#define R_RECV_BUFFER_SIZE 4096

/** Time to wait for the file info packet. */
#define R_FILE_INFO_TIMEOUT_MS 1000

/**
 * Defines whether the receiver should save received data to a temporary file.
 * When set to 0, memory will be allocated to store all the data.
 */
#define R_USE_TEMP_FILE 0

/* ------ Protocol constants ------ */

#define PROTO_HELLO { 0x01, 0x10, 0x01, 0x10 }
#define PROTO_HELLO_BACK { 0x10, 0x01, 0x10, 0x01 }
#define PROTO_OK 0x1
#define PROTO_ERROR 0x2
#define PROTO_INVALID_KEY 0x3
#define PROTO_REQUEST_RESEND 0x4

// The maximum packet size must not be higher than the receive buffer size
static_assert(STARTING_MPS <= R_RECV_BUFFER_SIZE, "The maximum packet size must not be higher than the receive buffer size.");

#endif //ISA_COMMON_H
