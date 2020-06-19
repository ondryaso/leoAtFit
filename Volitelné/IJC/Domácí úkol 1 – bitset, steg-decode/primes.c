// primes.c
// Řešení IJC-DU1
// příklad A)
// Datum: 13. 3. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT

#include <time.h>
#include "eratosthenes.h"

int main() {
    unsigned int take = ERATOSTHENES_DEFAULT_TAKE;

    clock_t start = clock();

    bitset_create(b, ERATOSTHENES_DEFAULT_N);
    Eratosthenes(b);

    unsigned long taken[ERATOSTHENES_DEFAULT_TAKE];

    for(bitset_index_t i = ERATOSTHENES_DEFAULT_N - 1; i > 0 && take > 0; i--) {
        if(bitset_getbit(b, i) == 0) {
            taken[take - 1] = i;
            take--;
        }
    }

    for(size_t i = 0; i < ERATOSTHENES_DEFAULT_TAKE; i++) {
        printf("%lu\n", taken[i]);
    }

    fprintf(stderr, "Time=%.3g\n", (double) (clock() - start) / CLOCKS_PER_SEC);

    return 0;
}
