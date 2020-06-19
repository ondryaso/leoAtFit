// wordcount.c
// Řešení IJC-DU2
// příklad B
// Datum: 17. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Program vrací kód ERRC_LIMIT (1), pokud dojde k překročení implementačního limitu na maximální délku slova.
// Program vrací kód ERRC_FILE (2), pokud dojde k chybě při čtení vstupu.
//
// Řešení využívá povolený implementační limit na maximální délku slova (127 znaků + \0). (Vzhledem k požadavku
// na funkci get_word s param. max to ani jinak logicky nedává smysl.)
// Jako velikost pole hashtable jsem zvolil 8 192 prvků (HTAB_BUCKETS). Zvyšování počtu buckets má na výkon výrazný
// vliv, protože přímý přístup na index v poli je výrazně rychlejší než procházení vázaného seznamu a ZEJMÉNA
// porovnávání klíčů řetězců (na metodě strcmp vzniká nejvíce cache misses). Rychlost jsem
// měřil na seznamu 1 000 000 unikátních čísel a na textu románu Pride and Prejudice, který má 13 669 unikátních slov.
// Pro 1 000 000 unikátních slov se časové rozdíly projeví výrazně, na běžném textu, ve kterém se slova skutečně
// opakují, se však rozdíl mezi např. 8 192 a 32 768 buckets prakticky neprojeví. Samozřejmě, paměti je dost a použití
// vyšší hodnoty se může hodit, soudím ale, že pro typické texty je 8 192 prvků naprosto dostatečné množství.

#include <stdio.h>
#include <stdbool.h>
#include "htab.h"
#include "io.h"

#define WORD_LIMIT 128
#define HTAB_BUCKETS 8192

// The error code to return when the word length limit is exceeded
#define ERRC_LIMIT 1
// The error code to return when an IO error occurs
#define ERRC_FILE 2

// If HASHTEST is defined, this very dumb hashing function that groups words based on their length and
// their first letter is used instead of the library one.
// (For a text with reasonable distribution of words and their lengths, it not _that_ terrible.)
#ifdef HASHTEST
size_t htab_hash_fun(htab_key_t str) {
    return strlen(str) * (1 + str[0] / 10);
}
#endif

int main() {
    htab_t *m = htab_init(HTAB_BUCKETS);

    char buf[WORD_LIMIT];
    int readRes = 0;
    bool warned = false;

    while ((readRes = get_word(buf, WORD_LIMIT, stdin)) != EOF) {
        if (readRes == GW_FERROR) {
            perror("Couldn't read the file");
            htab_end(m);
            return ERRC_FILE;
        }

        if (readRes == GW_TOOLONG && !warned) {
            fprintf(stderr, "Word length limit (%d characters) exceeded.\n", WORD_LIMIT);
            warned = true;
        }

        if (readRes == 0) {
            // Skip empty words.
            continue;
        }

        htab_iterator_t it = htab_lookup_add(m, buf);
        int val = htab_iterator_get_value(it);
        htab_iterator_set_value(it, val + 1);
    }

    htab_iterator_t it = htab_begin(m);
    while (htab_iterator_valid(it)) {
        printf("%s\t%d\n", htab_iterator_get_key(it), htab_iterator_get_value(it));
        it = htab_iterator_next(it);
    }

    htab_free(m);
    return warned ? ERRC_LIMIT : 0;
}