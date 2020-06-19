// steg-decode.c
// Řešení IJC-DU1
// příklad B)
// Datum: 13. 3. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT

#include "ppm.h"
#include "bitset.h"
#include "eratosthenes.h"
#include <stdio.h>

char *find_message(ppm_t img) {
    size_t size = img->xsize * img->ysize * 3;

    bitset_alloc(primes, size + 1);
    size_t primeCount = Eratosthenes_find(primes);

    unsigned char *chars = calloc(primeCount, sizeof(unsigned char));
    size_t pos = 0;
    unsigned char lastCharCounter = 0;

    for (bitset_index_t i = 23; i < size; i++) {
        if (bitset_getbit(primes, i) == 0) {
            unsigned char imgByte = img->data[i];
            // The char is always zeroed at the beginning, so we can just OR in the ones where they're needed
            chars[pos] |= ((imgByte & 1U) << (lastCharCounter++));

            if (lastCharCounter == 8) {
                if (chars[pos] == '\0') {
                    bitset_free(primes);
                    return (char *) chars;
                }

                pos++;
                lastCharCounter = 0;
            }
        }
    }

    bitset_free(primes);
    free(chars);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc > 1) {
        ppm_t img = ppm_read(argv[1]);
        char *msg = find_message(img);
        ppm_free(img);

        if (!msg) {
            error_exit("Zakódovaná zpráva nebyla správně ukončena.\n");
        }

        printf("%s", msg);
        free(msg);

        return 0;
    } else {
        warning_msg("Neplatný počet argumentů. Použití: %s [cesta k PPM souboru]\n", argv[0]);
        return 1;
    }
}