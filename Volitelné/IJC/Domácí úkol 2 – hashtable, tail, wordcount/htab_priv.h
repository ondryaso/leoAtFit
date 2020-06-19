// htab_priv.h
// Řešení IJC-DU2
// příklad B
// Datum: 12. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Tento soubor obsahuje úplné deklarace struktur htab a htab_item, které jsou ve veřejném API
// (hlavičkovém souboru htab.h) deklarovány neúplně, pro uživatele nejsou transparentní.
// Obsahuje také deklarace interních funkcí.

#ifndef __HTAB_PRIV_H__
#define __HTAB_PRIV_H__

#include "htab.h"

struct htab_item {
    htab_key_t key;
    htab_value_t value;
    struct htab_item *next;
};

struct htab {
    size_t size;
    size_t arr_size;
    struct htab_item *data[];
};

typedef struct htab_item htab_item_t;

htab_iterator_t htab_insert(htab_t *t, htab_key_t key, size_t idx);

htab_iterator_t htab_find_common(htab_t *t, htab_key_t key, bool addIfNull);

#endif //__HTAB_PRIV_H__