// htab_test_extern.c
// Řešení IJC-DU2
// příklad B
// Datum: 12. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Tento soubor obsahuje externí definice inline funkcí z hlavičkového souboru htab.h,
// aby byl projekt zkompilovatelný i se zakázanými optimalizacemi.

#include <stdbool.h>
#include "htab.h"

extern bool htab_iterator_valid(htab_iterator_t it);
extern bool htab_iterator_equal(htab_iterator_t it1, htab_iterator_t it2);