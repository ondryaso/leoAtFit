// eratosthenes.h
// Řešení IJC-DU1
// příklad A)
// Datum: 13. 3. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT

#ifndef ERATOSTHENES_H
#define ERATOSTHENES_H

#include "bitset.h"

#define ERATOSTHENES_DEFAULT_N 500000001
#define ERATOSTHENES_DEFAULT_TAKE 10

void Eratosthenes(bitset_t bitset);
unsigned long Eratosthenes_find(bitset_t bitset);

#endif
