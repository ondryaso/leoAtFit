// htab_erase.c
// Řešení IJC-DU2
// příklad B
// Datum: 12. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Modul obsahuje definici funkce htab_erase.

#include <stdlib.h>
#include "htab.h"
#include "htab_priv.h"

void htab_erase(htab_t *t, htab_iterator_t it) {
    if (it.t != t) return;
    if (t == NULL) return;

    // We need to find the *previous item* to relink it
    // Using a doubly-linked list would probably be faster, especially for hashtables with small number of buckets.
    htab_item_t *bucket = t->data[it.idx];

    if (bucket == NULL) {
        // Sanity check, this should never happen for a valid iterator.
        return;
    }

    if (bucket == it.ptr) {
        // This item is the first one.
        t->data[it.idx] = it.ptr->next;
        t->size--;
        free((void *) it.ptr->key);
        free(it.ptr);
        return;
    }

    while (!(bucket->next == it.ptr || bucket == NULL)) {
        bucket = bucket->next;
    }

    if (bucket == NULL) {
        // Sanity check, this should again never happen for a valid iterator.
        return;
    }

    bucket->next = it.ptr->next;
    free((void *) it.ptr->key);
    free(it.ptr);
    t->size--;
}