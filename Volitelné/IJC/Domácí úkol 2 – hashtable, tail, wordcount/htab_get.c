// htab_get.c
// Řešení IJC-DU2
// příklad B
// Datum: 12. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Modul obsahuje definici funkcí htab_iterator_get_key a htab_iterator_get_value.

#include "htab.h"
#include "htab_priv.h"

htab_key_t htab_iterator_get_key(htab_iterator_t it) {
    if (it.ptr == NULL || it.t == NULL) {
        // Invalid iterator.
        return NULL;
    }

    return it.ptr->key;
}

htab_value_t htab_iterator_get_value(htab_iterator_t it) {
    if (it.ptr == NULL || it.t == NULL) {
        // Invalid iterator, return the default value.
        return 0;
    }

    return it.ptr->value;
}