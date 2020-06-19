// htab_insert_priv.c
// Řešení IJC-DU2
// příklad B
// Datum: 12. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Modul obsahuje definici interní (pro knihovnu privátní) funkce htab_insert, kterou využívá
// funkce htab_lookup_add (resp. htab_find_common) pro přidání nového záznamu do hashtable.

#include <stdlib.h>
#include "htab.h"
#include "htab_priv.h"

// Creates a new entry in the hashtable *t.
// Returns an iterator pointing to the entry or an end iterator if adding was unsuccessful (when malloc fails).
htab_iterator_t htab_insert(htab_t *t, htab_key_t key, size_t idx) {
    htab_item_t *newItem = malloc(sizeof(htab_item_t));

    if (newItem == NULL) {
        // malloc failed, return an end iterator
        return htab_end(t);
    }

    newItem->next = NULL;

    // Make a copy of the key string (the original might've been a changing buffer).
    size_t keyLen = strlen(key);
    char *keyCpy = malloc(sizeof(char) * keyLen + 1);
    if (keyCpy == NULL) {
        // malloc failed, free the newItem and return an end iterator
        free(newItem);
        return htab_end(t);
    }
    strcpy(keyCpy, key);

    newItem->key = keyCpy;
    newItem->value = 0;

    // Assign the new entry to the hashtable
    htab_item_t *bucket = t->data[idx];
    if (bucket == NULL) {
        // The bucket is empty, make this the first entry in it
        t->data[idx] = newItem;
    } else {
        // Move to the last entry in the chain starting at the bucket
        while (bucket->next != NULL) {
            bucket = bucket->next;
        }

        bucket->next = newItem;
    }

    t->size++;
    htab_iterator_t ret = {.idx = idx, .ptr = newItem, .t = t};
    return ret;
}
