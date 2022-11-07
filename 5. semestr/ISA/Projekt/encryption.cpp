// encryption.cpp
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)
//
// This implementation of encryption/decryption is inspired by:
// - https://github.com/saju/misc/blob/master/misc/openssl_aes.c by Saju Pillai (saju.pillai@gmail.com)
// - https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption (OpenSSL wiki)

#include <openssl/aes.h>
#include <limits>
#include <system_error>

#include "common.h"
#include "encryption.h"
#include "secure_string.h"


Crypto::Crypto(const secure_string &password, uint32_t salt) {
    EVP_add_cipher(EVP_aes_256_cbc());

    auto salt_ptr = reinterpret_cast<const unsigned char *>(&salt);

    int key_len = EVP_CIPHER_key_length(EVP_aes_256_cbc());
    int iv_len = EVP_CIPHER_iv_length(EVP_aes_256_cbc());
    int key_iv_len = key_len + iv_len;

    unsigned char key_iv[key_iv_len];

    if (!PKCS5_PBKDF2_HMAC_SHA1(password.c_str(), static_cast<int>(password.length()), salt_ptr, sizeof(salt),
                                CRYPTO_KEY_DERIVATION_ITERATIONS, key_iv_len, key_iv)) {
        throw std::runtime_error("Cannot create encryption parameters.");
    }

    encrypt_ctx = EVP_CIPHER_CTX_new();
    decrypt_ctx = EVP_CIPHER_CTX_new();

    if (encrypt_ctx == nullptr || decrypt_ctx == nullptr) {
        throw std::runtime_error("Cannot initialize OpenSSL contexts.");
    }

    int success = 1;

    success &= EVP_CIPHER_CTX_reset(encrypt_ctx);
    success &= EVP_CIPHER_CTX_reset(decrypt_ctx);

    success &= EVP_EncryptInit_ex(encrypt_ctx, EVP_aes_256_cbc(), nullptr, key_iv, key_iv + key_len);
    success &= EVP_DecryptInit_ex(decrypt_ctx, EVP_aes_256_cbc(), nullptr, key_iv, key_iv + key_len);

    OPENSSL_cleanse(key_iv, key_iv_len);

    if (!success) {
        throw std::runtime_error("Cannot initialize OpenSSL contexts.");
    }
}

size_t Crypto::encrypt(unsigned char *dest, const unsigned char *input_plain, size_t input_len, bool is_final) {
    int len_i = static_cast<int>(input_len);
    int out_len = len_i + AES_BLOCK_SIZE;
    int final_len;

    if (!EVP_EncryptUpdate(encrypt_ctx, dest, &out_len, input_plain, len_i)) {
        throw std::runtime_error("Cannot encrypt data.");
    }

    if (is_final) {
        if (!EVP_EncryptFinal_ex(encrypt_ctx, dest + out_len, &final_len)) {
            throw std::runtime_error("Cannot encrypt data.");
        }

        return out_len + final_len;
    } else {
        return out_len;
    }
}

size_t Crypto::decrypt(unsigned char *dest, const unsigned char *input_ciphertext, size_t input_len, bool is_final) {
    int len_i = static_cast<int>(input_len);
    int final_len;

    if (!EVP_DecryptUpdate(decrypt_ctx, dest, &len_i, input_ciphertext, len_i)) {
        throw std::runtime_error("Cannot decrypt data");
    }

    if (is_final) {
        if (!EVP_DecryptFinal_ex(decrypt_ctx, dest + len_i, &final_len)) {
            throw std::runtime_error("Cannot decrypt data");
        }

        return len_i + final_len;
    } else {
        return len_i;
    }
}

Crypto::~Crypto() {
    EVP_CIPHER_CTX_free(encrypt_ctx);
    EVP_CIPHER_CTX_free(decrypt_ctx);
}

size_t Crypto::max_plain_len_to_fit(size_t limit_len) {
    return limit_len - AES_BLOCK_SIZE;
}

size_t Crypto::encrypted_len(size_t input_len) {
    // AES has a fixed block size of 16 B
    // There will always be padding at the end – if the input size is a multiple of 16,
    // a whole new block is needed for padding
    return (input_len / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
}
