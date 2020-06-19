// htab_begin.c
// Řešení IJC-DU2
// příklad B
// Datum: 12. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Modul obsahuje definici funkce htab_begin.

#include "htab.h"
#include "htab_priv.h"

htab_iterator_t htab_begin(const htab_t *t) {
    htab_iterator_t ret;
    ret.t = t;

    if (t == NULL) {
        // Return an invalid iterator.
        ret.ptr = NULL;
        ret.idx = -1;
        return ret;
    }

    // Find the first non-empty bucket.
    size_t idx = 0;
    htab_item_t *bucket = NULL;
    while (!(bucket != NULL || idx == t->arr_size)) {
        bucket = t->data[idx++];
    }

    if (bucket == NULL) {
        // The table is empty.
        return htab_end(t);
    }

    ret.ptr = bucket;
    ret.idx = idx - 1;
    return ret;
}

