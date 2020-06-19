// htab_hash_fun.c
// Řešení IJC-DU2
// příklad B
// Datum: 12. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Modul obsahuje definici hashovací funkce htab_hash_fun (podle zadání).

#include <stdint.h>
#include "htab.h"

size_t htab_hash_fun(const char *str) {
    uint32_t h = 0;
    const unsigned char *p;

    for (p = (const unsigned char *) str; *p != '\0'; p++) {
        h = 65599 * h + *p;
    }

    return h;
}