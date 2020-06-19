// io.c
// Řešení IJC-DU2
// příklad B
// Datum: 18. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Modul obsahuje definici funkce get_word.

#include <ctype.h>

#include "io.h"

// Reads a word from the stream *f and stores the first (max-1) characters it in the buffer *s.
// If there are more than (max-1) characters, all the characters until the next space are discarded
// and GW_TOOLONG is returned. The whitespace is omitted. A terminating \0 is always appended at the end.
// A word is a sequence of any characters ended by a whitespace.
// Returns the number of characters read, EOF or one of the GW_ status codes.
int get_word(char *s, int max, FILE *f) {
    if (s == NULL || f == NULL) return GW_INVALID_ARGUMENT;
    if (feof(f)) return EOF;
    if (max < 0) return GW_INVALID_ARGUMENT;
    if (max == 0) return GW_INVALID_ARGUMENT;
    if (max == 1) {
        s[0] = '\0';
        return 0;
    }

    int read;
    char *begin = s;
    s[max - 1] = '\0';

    while (max-- > 1) {
        read = fgetc(f);

        if (read == EOF) {
            // If we reached an erroneous state, return the error flag,
            // otherwise, it's an indecent file that doesn't end with an EOL, let's take what's been read
            // as a word and roll with it.
            if (ferror(f)) return GW_FERROR;

            *(s++) = '\0';
            return (int) (s - begin - 1);
        }

        *(s++) = (char) read;
        if (isspace(*(s - 1))) {
            *(s - 1) = '\0';
            return (int) (s - begin - 1);
        }
    }

    // Skip the rest
    while (!isspace(fgetc(f)));
    return GW_TOOLONG;
}