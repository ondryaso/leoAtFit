// htab_find_priv.c
// Řešení IJC-DU2
// příklad B
// Datum: 12. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Modul obsahuje definici interní (pro knihovnu privátní) funkce htab_find_common,
// která vyhledává záznam v tabulce a pokud jej nenajde, buď jej přidá a vrátí, nebo vrátí end iterator.
// Funkce je využita pro veřejné funkce htab_find a htab_lookup_add.

#include <stdbool.h>
#include "htab.h"
#include "htab_priv.h"

// Looks up the item with specified key in the hashtable *t.
// In case it's not found, depending on the value of addIfNull, it either creates the entry
// or returns an end iterator.
htab_iterator_t htab_find_common(htab_t *t, htab_key_t key, bool addIfNull) {
    if(t == NULL) {
        // An invalid iterator.
        htab_iterator_t ret = { .ptr = NULL, .t = NULL, .idx = -1 };
        return ret;
    }

    size_t hash = htab_hash_fun(key);
    size_t index = hash % t->arr_size;

    struct htab_item *item = t->data[index];

    while (item != NULL) {
        // Hashing won't yield unique values, compare the keys.
        if (strcmp(key, item->key) == 0) {
            htab_iterator_t ret = {.ptr = item, .t = t, .idx = index};
            return ret;
        } else {
            item = item->next;
        }
    }

    if (addIfNull) {
        return htab_insert(t, key, index);
    } else {
        return htab_end(t);
    }
}