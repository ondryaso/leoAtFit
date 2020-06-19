// htab_iterator_next.c
// Řešení IJC-DU2
// příklad B
// Datum: 12. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Modul obsahuje definici funkce htab_iterator_next.

#include "htab.h"
#include "htab_priv.h"

htab_iterator_t htab_iterator_next(htab_iterator_t it) {
    htab_iterator_t ret;
    ret.t = it.t;

    if (it.t == NULL) {
        // Invalid iterator, return another invalid iterator.
        ret.ptr = NULL;
        ret.idx = -1;
        return ret;
    }

    if (it.ptr == NULL) {
        // End iterator or an invalid iterator with a htab reference, return a new end iterator.
        return htab_end(it.t);
    }

    if (it.ptr->next != NULL) {
        // We are in the same bucket.
        ret.ptr = it.ptr->next;
        ret.idx = it.idx;
        return ret;
    }

    // Find the next non-empty bucket.
    ret.idx = it.idx + 1;
    while (ret.idx < it.t->arr_size) {
        if (it.t->data[ret.idx] != NULL) {
            break;
        }
        ret.idx++;
    }

    if (ret.idx == it.t->arr_size) {
        // We have reached the end of the table and haven't found a filled bucket.
        return htab_end(it.t);
    }

    ret.ptr = it.t->data[ret.idx];
    return ret;
}