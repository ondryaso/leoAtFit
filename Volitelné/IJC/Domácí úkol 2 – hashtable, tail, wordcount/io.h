// io.h
// Řešení IJC-DU2
// příklad B
// Datum: 12. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Hlavičkový soubor s deklarací funkce get_word (podle zadání).

#ifndef __IO_H__
#define __IO_H__

#include <stdio.h>

enum GetWordError {
    GW_INVALID_ARGUMENT = -2,
    GW_FERROR = -3,
    GW_TOOLONG = -4
};
int get_word(char *s, int max, FILE *f);

#endif //__IO_H__
