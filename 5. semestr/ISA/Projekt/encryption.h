// encryption.h
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)


#ifndef ISA_ENCRYPTION_H
#define ISA_ENCRYPTION_H

#include <string>
#include <openssl/evp.h>

#include "secure_string.h"

using std::string;

/**
 * A Crypto instance is used to encrypt or decrypt a single logical unit of data (e.g. one file) using
 * given password and salt and the AES_256_CBC cipher.
 */
class Crypto {
public:
    /**
     * Creates a Crypto instance. Generates encryption parameters based on the provided password and salt.
     * Initializes both the encryption and decryption context.
     *
     * @remark Both the password and salt must be known to the other side.
     * @param password A password to generate the encryption password and IV from.
     * @param salt A salt to bring out the flavours of the encryption password and IV.
     */
    Crypto(const secure_string &password, uint32_t salt);

    /**
     * Frees resources used by this Crypto instance.
     */
    ~Crypto();

    /**
     * Encrypts a chunk of data.
     * @remark Successive calls to encrypt() encrypt successive chunks of a single data stream.
     * @remark After the final chunk is encrypted (with is_final set), this Crypto instance should no longer
     * be used for encrypting more data.
     * @param [out] dest A pointer to a buffer to save the encrypted data to.
     * @param [in] input_plain A pointer to the input data.
     * @param [in] input_len Length of the input data.
     * @param [in] is_final Must be set to true when this chunk is the last chunk of encrypted data and
     * to false otherwise. Used to properly align the data to encrypted blocks.
     * @return Number of bytes saved to the dest buffer.
     */
    size_t encrypt(unsigned char *dest, const unsigned char *input_plain, size_t input_len, bool is_final = false);

    /**
     * Decrypts a chunk of data.
     * @remark Successive calls to decrypt() decrypt successive chunks of a single data stream.
     * @remark After the final chunk is decrypted (with is_final set), this Crypto instance should no longer
     * be used for decrypting more data.
     * @param [out] dest A pointer to a buffer to save the decrypted data to.
     * @param [in] input_plain A pointer to the input data.
     * @param [in] input_len Length of the input data.
     * @param [in] is_final Must be set to true when this chunk is the last chunk of decrypted data and
     * to false otherwise. Used to properly handle alignment of the encrypted blocks.
     * @return Number of bytes saved to the dest buffer.
     */
    size_t decrypt(unsigned char *dest, const unsigned char *input_ciphertext, size_t input_len, bool is_final = false);

    /**
     * Calculates the maximum number of bytes that encrypt() may save to the destination buffer for the specified
     * number of input bytes.
     * @param [in] input_len The number of input (encrypted) bytes.
     * @return The maximum length of encrypted data.
     */
    static size_t encrypted_len(size_t input_len);

    /**
     * Calculates the maximum possible number of bytes that can be used as input for encrypt() so that it doesn't
     * save more than limit_len bytes to the destination buffer.
     * @param [in] limit_len The maximum size of the destination buffer in bytes.
     * @return The maximum possible length of input data.
     */
    static size_t max_plain_len_to_fit(size_t limit_len);

private:
    EVP_CIPHER_CTX *encrypt_ctx; /**< Encryption context. */
    EVP_CIPHER_CTX *decrypt_ctx; /**< Decryption context. */

};

#endif //ISA_ENCRYPTION_H
