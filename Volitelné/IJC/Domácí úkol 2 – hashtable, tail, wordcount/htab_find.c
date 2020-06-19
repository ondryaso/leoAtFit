// htab_find.c
// Řešení IJC-DU2
// příklad B
// Datum: 12. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Modul obsahuje definici funkce htab_find.
// Funkcionalita této funkce je prakticky shodná s funkcí htab_lookup_add, používají tak společnou funkci
// htab_find_common definovanou ve stejnojmenném modulu.

#include <stdbool.h>
#include "htab.h"
#include "htab_priv.h"

htab_iterator_t htab_find(htab_t *t, htab_key_t key) {
    return htab_find_common(t, key, false);
}

