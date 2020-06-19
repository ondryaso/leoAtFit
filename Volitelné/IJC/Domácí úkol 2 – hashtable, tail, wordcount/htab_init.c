// htab_init.c
// Řešení IJC-DU2
// příklad B
// Datum: 14. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Modul obsahuje definici konstruktoru hashtable, funkce htab_init.

#include <stdlib.h>
#include "htab.h"
#include "htab_priv.h"

htab_t *htab_init(size_t n) {
    if (n == 0) {
        return NULL;
    }

    htab_t *htabMem = malloc(sizeof(htab_t) + sizeof(htab_item_t *) * n);
    if (htabMem == NULL) {
        return NULL;
    }

    // Initialise the array with NULLs (this wouldn't be entirely correct on some weird platforms
    // where 0 != NULL, but it wouldn't matter that much anyway.)
    memset(htabMem->data, 0, sizeof(htab_item_t *) * n);

    htabMem->size = 0;
    htabMem->arr_size = n;
    return htabMem;
}

