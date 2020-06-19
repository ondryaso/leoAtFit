// bitset.h
// Řešení IJC-DU1
// příklad A)
// Datum: 13. 3. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
//
// Poznámky:
// Makro bitset_alloc nepoužívá assert() pro ověření, jestli není vrácený ukazatel NULL,
// aby mohlo být přesně dodrženo chybové hlášení ze zadání. Pro vynulování pole je použita
// funkce calloc(), která je efektivnější než ruční nulování.

#ifndef BITSET_H
#define BITSET_H

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include "error.h"

#define BPC (sizeof(unsigned long)*CHAR_BIT) // Bits per ulong

typedef unsigned long bitset_index_t;
typedef unsigned long *bitset_t;

#define BITSET_SIZE_STATIC_ASSERT(size) static_assert((size) > 0, "Neplatná velikost pole.")
#define BITSET_SIZE_ASSERT(size) assert((size) > 0 && "Neplatná velikost pole.")

#define BITSET_SIZE(size) (((size) / BPC) + ((size) % BPC == 0 ? 1UL : 2UL))
#define BITSET_BYTE_INDEX(index) (1 + (index)/BPC)
#define BITSET_BIT_OFFSET(index) ((index) % BPC)

#define bitset_create(name, size) unsigned long name[BITSET_SIZE(size)] = {(size)}; BITSET_SIZE_STATIC_ASSERT(size)
#define bitset_alloc(name, size) bitset_t name; BITSET_SIZE_ASSERT(size); \
    name = calloc(BITSET_SIZE(size), sizeof(unsigned long)); \
    if(name == NULL) { fprintf(stderr, "bitset_alloc: Chyba alokace paměti"); exit(1); } \
    name[0] = size

#ifdef USE_INLINE
#define BITSET_CHECK_BOUNDS(bitset, index)  if ((index) >= (bitset)[0]) { \
    error_exit("bitset_getbit: Index %lu mimo rozsah 0..%lu", (index), (bitset)[0] - 1); }

static inline void bitset_free(bitset_t bitset) {
    if (bitset) free(bitset);
}

static inline size_t bitset_size(bitset_t bitset) {
    return bitset[0];
}

static inline void bitset_setbit(bitset_t bitset, bitset_index_t index, int expression) {
    BITSET_CHECK_BOUNDS(bitset, index);

    // Using an if is faster than calculating "complex" bit masks.
    if (expression) {
        bitset[BITSET_BYTE_INDEX(index)] |= (1UL << BITSET_BIT_OFFSET(index));
    } else {
        bitset[BITSET_BYTE_INDEX(index)] &= ~(1UL << BITSET_BIT_OFFSET(index));
    }
}

static inline unsigned char bitset_getbit(bitset_t bitset, bitset_index_t index) {
    BITSET_CHECK_BOUNDS(bitset, index);
    return ((bitset[BITSET_BYTE_INDEX(index)] >> BITSET_BIT_OFFSET(index)) & 1U);
}

#else
#define BITSET_CHECK_BOUNDS(name, index) ((index < name[0]) ? 1 : \
    (error_exit("bitset_getbit: Index %lu mimo rozsah 0..%lu", (unsigned long)index, (unsigned long)name[0] - 1), 0))

#define bitset_free(name) do { if (name) free(name); } while(0)
#define bitset_size(name) name[0]

#define bitset_setbit(name, index, expression) do { BITSET_CHECK_BOUNDS(name, index); \
    if (expression) { name[BITSET_BYTE_INDEX(index)] |= (1UL << BITSET_BIT_OFFSET(index)); } \
    else { name[BITSET_BYTE_INDEX(index)] &= ~(1UL << BITSET_BIT_OFFSET(index)); } } while(0)

#define bitset_getbit(name, index) (BITSET_CHECK_BOUNDS(name, index), ((name[BITSET_BYTE_INDEX(index)] >> BITSET_BIT_OFFSET(index))) & 1U)
#endif
#endif