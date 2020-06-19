// htab_clear.c
// Řešení IJC-DU2
// příklad B
// Datum: 14. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Modul obsahuje definici funkce htab_clear.

#include <stdlib.h>
#include "htab.h"
#include "htab_priv.h"

void htab_clear(htab_t *t) {
    if (t == NULL) {
        return;
    }

    htab_iterator_t iter = htab_begin(t);

    while (iter.ptr != NULL) {
        htab_item_t *toFree = iter.ptr;
        iter = htab_iterator_next(iter);
        free((void *) toFree->key);
        free(toFree);
    }

    t->size = 0;
}

