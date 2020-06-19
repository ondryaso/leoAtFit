// htab_size.c
// Řešení IJC-DU2
// příklad B
// Datum: 12. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Modul obsahuje definici funkcí htab_size a htab_bucket_count. Rozdělovat jej mezi dva moduly mi přišlo,
// vzhledem k jejich rozsahu, zcela zbytečné.

#include "htab.h"
#include "htab_priv.h"

size_t htab_size(const htab_t *t) {
    return t->size;
}
size_t htab_bucket_count(const htab_t *t) {
    return t->arr_size;
}