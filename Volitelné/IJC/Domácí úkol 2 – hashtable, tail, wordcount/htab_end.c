// htab_end.c
// Řešení IJC-DU2
// příklad B
// Datum: 12. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Modul obsahuje definici funkce htab_end.

#include "htab.h"
#include "htab_priv.h"

htab_iterator_t htab_end(const htab_t *t) {
    htab_iterator_t ret = {.idx = -1, .ptr = NULL, .t = t};
    return ret;
}