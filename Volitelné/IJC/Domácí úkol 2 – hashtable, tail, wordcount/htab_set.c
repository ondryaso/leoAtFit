// htab_set.c
// Řešení IJC-DU2
// příklad B
// Datum: 12. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Modul obsahuje definici funkce htab_iterator_set_value.

#include "htab.h"
#include "htab_priv.h"

htab_value_t htab_iterator_set_value(htab_iterator_t it, htab_value_t val) {
    if(it.ptr == NULL || it.t == NULL) {
        // Invalid iterator, return a value different from val, preferably zero.
        return val == 0 ? -1 : 0;
    }

    it.ptr->value = val;
    return val;
}
