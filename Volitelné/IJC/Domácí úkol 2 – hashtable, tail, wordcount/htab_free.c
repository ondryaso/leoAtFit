// htab_free.c
// Řešení IJC-DU2
// příklad B
// Datum: 14. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Modul obsahuje definici destruktoru hashtable, funkce htab_free.

#include <stdlib.h>
#include "htab.h"
#include "htab_priv.h"

void htab_free(htab_t *t) {
    if (t == NULL) {
        return;
    }

    htab_clear(t);
    free(t);
}
