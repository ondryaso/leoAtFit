// secure_string.h
// Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)


#ifndef ISA_SECURE_STRING_H
#define ISA_SECURE_STRING_H

#include <openssl/crypto.h>
#include <limits>

// A 'secure string' allocator that cleans memory after deallocating
// This is copied from the OpenSSL wiki example
// https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
// https://wiki.openssl.org/images/5/5d/Evp-encrypt-cxx.tar.gz
// Copyright OpenSSL 2021
// Contents licensed under the terms of the OpenSSL license
// See https://www.openssl.org/source/license.html for details
template<typename T>
struct zeroing_allocator {
public:
    typedef T value_type;
    typedef value_type *pointer;
    typedef const value_type *const_pointer;
    typedef value_type &reference;
    typedef const value_type &const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    pointer address(reference v) const { return &v; }

    const_pointer address(const_reference v) const { return &v; }

    pointer allocate(size_type n, const void *hint = nullptr) {
        if (n > std::numeric_limits<size_type>::max() / sizeof(T))
            throw std::bad_alloc();
        return static_cast<pointer> (operator new(n * sizeof(value_type)));
    }

    void deallocate(pointer p, size_type n) {
        OPENSSL_cleanse(p, n * sizeof(T));
        ::operator delete(p);
    }

    [[nodiscard]] size_type max_size() const {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    template<typename U>
    struct rebind {
        typedef zeroing_allocator<U> other;
    };

    void construct(pointer ptr, const T &val) {
        new(static_cast<T *>(ptr)) T(val);
    }

    void destroy(pointer ptr) {
        static_cast<T *>(ptr)->~T();
    }

#if __cpluplus >= 201103L
    template<typename U, typename... Args>
    void construct (U* ptr, Args&&  ... args) {
        ::new (static_cast<void*> (ptr) ) U (std::forward<Args> (args)...);
    }

    template<typename U>
    void destroy(U* ptr) {
        ptr->~U();
    }
#endif
};

typedef std::basic_string<char, std::char_traits<char>, zeroing_allocator<char> > secure_string;

#endif //ISA_SECURE_STRING_H
