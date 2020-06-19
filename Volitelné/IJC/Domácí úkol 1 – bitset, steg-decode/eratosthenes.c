// eratosthenes.c
// Řešení IJC-DU1
// příklad A)
// Datum: 13. 3. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
//
// Poznámka:
// Zadání vyžaduje funkci s hlavičkou void Eratosthenes(...).
// Kvůli optimalizaci při použití v příkladu B je pro mne výhodné,
// aby funkce vracela počet nalezených prvočísel. Aby bylo zadání
// splněno, síto jsem implementoval ve funkci Eratosthenes_find,
// která vrací unsigned long, a funkce Eratosthenes ji pouze zavolá.
// S touto zbytečnou redundancí nejsem spokojen, ale podle dostupných
// informací hodnocení vyžaduje přesné dodržení zadaných hlaviček.

#include <stdio.h>
#include <math.h>

#include "eratosthenes.h"

void Eratosthenes(bitset_t bitset) {
    Eratosthenes_find(bitset);
}

unsigned long Eratosthenes_find(bitset_t bitset) {
    if(bitset == NULL) return 0;

    unsigned long max = bitset_size(bitset);

    // Max will not be zero. It might be one though.
    if(max == 1) {
        bitset_setbit(bitset, 0, 1);
        return 1;
    }

    // Making a floor by simply casting to uint would make
    // the algorithm skip the last prime for small ranges.
    // The difference between calling ceil and casting to uint
    // and adding a 1 "just in case" is basically unmeasurable.
    unsigned long maxSqrt = (unsigned long) ceil(sqrt((double) max));
    unsigned long found = 2;

    bitset_setbit(bitset, 0, 1);
    bitset_setbit(bitset, 1, 1);

    // Bitset is zeroed by default
    for (bitset_index_t i = 2; i < maxSqrt; i++) {
        if (bitset_getbit(bitset, i) == 0) {
            for (bitset_index_t a = i*i; a < max; a += i) {
                bitset_setbit(bitset, a, 1);
                found++;
            }
        }
    }

    return found;
}