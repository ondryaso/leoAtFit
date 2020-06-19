// tail.c
// Řešení IJC-DU2
// příklad A
// Datum: 18. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Program vrací kód ERRC_ARGS (1), pokud jsou mu předány neplatné argumenty.
// Program vrací kód ERRC_FILE (2), pokud dojde k chybě při čtení vstupu.
//
// > Program byl implementován s cílem nevyužít povolený implementační limit.
//   K načítání řádků využívá vlastní funkce read_line_dynamic, která ukládá načtené řetězce do bufferu
//   na heapu. Pokud zjistí, že nebyl načtený celý řádek (na konci načteného řetězce se nenachází \n),
//   realokuje buffer na dvakrát větší velikost a pokračuje ve čtení. Toto je prováděno, dokud není
//   celý řádek načten. V první verzi byl buffer nakonec realokován na velikost řetězce, aby byla uvol-
//   něna nevyužitá paměť, v rámci tohoto programu je to ale zbytečná operace navíc, protože je řádek
//   vždy brzy uvolněn. Pravděpodobně to není výrazně paměťově efektivní řešení, ale zde postačuje.
// > Funkce print_last_n_lines() využívá cyklicky přepisovaný buffer - pole N ukazatelů na načtené řádky.
// > Parsování argumentů nepoužívá getops(), protože většina kódu by i tak zůstala nezměněna a časově ná-
//   ročné studium syntaxe této funkce by tak bylo zbytečné.

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"

// The error code to return when the arguments are invalid
#define ERRC_ARGS 1
// The error code to return when an IO error occurs
#define ERRC_FILE 2

// Number of bytes allocated in every read_line_dynamic call. Must be greater than 1.
#define INITIAL_BUFFER_SIZE 64

// This function reads a variable-length line from the specified stream.
// The line is guaranteed to end with a '\n' character.
// It dynamically reallocates its buffer if the line is too long.
// A null is returned when the stream contains no more lines (it has reached EOF already)
// or when an allocation error occurs.
char *read_line_dynamic(FILE *s) {
    if (feof(s))
        return NULL;

    size_t currentSize = INITIAL_BUFFER_SIZE;
    size_t readSize = currentSize;

    char *buffer = malloc(currentSize);
    if (buffer == NULL) {
        return NULL;
    }

    char *readPtr = buffer;

    while (true) {
        // Set the last two elements to anything that's not "\n\0"
        buffer[currentSize - 1] = 1;
        buffer[currentSize - 2] = 1;

        char *ret = fgets(readPtr, readSize, s);

        if (ret == NULL) {
            // fgets returns NULL if EOF is reached and no chars could be read.
            // This state should never happen at this place as reaching EOF after a part of a line has been read
            // would trigger returning the buffer, so this basically a sanity check. If it happened, for some reason,
            // we can't tell what the buffer would look like, so we'll interpret it as an erroneous state.
            free(buffer);
            return NULL;
        }

        if (buffer[currentSize - 2] == '\n') {
            // A line spanning over the whole buffer has been read, we can return the current buffer
            return buffer;
        }

        if (buffer[currentSize - 1] == '\0') {
            // There were more characters in the line than there was space for
            // Realloc and read the next part
            readSize = currentSize + 1;
            char *newBuffer = realloc(buffer, currentSize *= 2);

            if (newBuffer == NULL) {
                // Can't alloc more memory.
                free(buffer);
                return NULL;
            }

            buffer = newBuffer;
            readPtr = newBuffer + readSize - 2;
        } else {
            // The string has been read and it's shorter than the current buffer size
            // Check whether it ends with a newline, if not, append it (the output should always end with a newline,
            // as stated by PePe on the forum)

            size_t len = strlen(buffer);
            if (buffer[len - 1] != '\n') {
                if ((currentSize - 1) == len) {
                    // There is no space for the new '\n' char in the buffer, we need to realloc

                    char *newBuffer = realloc(buffer, currentSize + 1);

                    if (newBuffer == NULL) {
                        // Can't alloc more memory.
                        free(buffer);
                        return NULL;
                    }

                    buffer = newBuffer;
                }

                buffer[len] = '\n';
                buffer[len + 1] = '\0';
            }

            return buffer;
        }
    }
}

void print_last_n_lines(unsigned int lineCount, FILE *s) {
    if (lineCount == 0) {
        return;
    }

    size_t keepLines = lineCount;
    size_t pos = 0; // Tracks the current position (the position that has been changed last)

    char *lines[keepLines]; // A cyclic buffer of pointers to loaded lines, initialised with NULLs
    for (size_t i = 0; i < keepLines; i++) {
        lines[i] = NULL;
    }

    char *line;
    while ((line = read_line_dynamic(s)) != NULL) {
        pos = (pos + 1) % keepLines;

        // If there's a line in the buffer on the current position,
        // free it from the memory and overwrite it with the currently read one.
        if (lines[pos] != NULL) {
            free(lines[pos]);
        }

        lines[pos] = line;
    }

    // Print the buffer, starting from (pos + 1), that is, the top-most line
    // (or NULL, which is skipped until we get to the top-most line).
    for (size_t i = 0; i < keepLines; i++) {
        line = lines[(pos + 1 + i) % keepLines];

        if (line == NULL) {
            continue;
        }

        fputs(line, stdout);
        free(line);
    }
}

void skip_n_lines(unsigned int lineCount, FILE *s) {
    unsigned int skipped = 0;

    char *line;
    while ((line = read_line_dynamic(s)) != NULL) {
        // Discard the first lineCount lines.
        if (skipped != lineCount) {
            skipped++;
            free(line);
            continue;
        }

        fputs(line, stdout);
        free(line);
    }
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        print_last_n_lines(10, stdin);
        return 0;
    }

    if (strcmp(argv[1], "-n") == 0) {
        if (argc == 2) {
            fprintf(stderr, "Invalid argument count. Usage: %s [-n (+)N] [path to file]\n", argv[0]);
            return ERRC_ARGS;
        }

        long num;
        bool skip = false;
        char *endPtr = NULL;

        if (*argv[2] == '+') {
            skip = true;
            num = strtol(argv[2] + 1, &endPtr, 10);
            if (num > 0) num--; // POSIX tail only skips n-1 lines.
        } else {
            num = strtol(argv[2], &endPtr, 10);
        }

        if (num < 0) {
            num = -num;
        }

        if (num == 0 && *endPtr != '\0') {
            fprintf(stderr, "%s is not a valid line count specifier.\n", argv[2]);
            return ERRC_ARGS;
        }

        if (!skip && num == 0) {
            return 0;
        }

        FILE *stream;
        if (argc >= 4) {
            stream = fopen(argv[3], "r");

            if (stream == NULL) {
                perror("Couldn't read the input file");
                return ERRC_FILE;
            }
        } else {
            stream = stdin;
        }

        if (skip) {
            skip_n_lines((unsigned int) num, stream);
        } else {
            print_last_n_lines((unsigned int) num, stream);
        }

        fclose(stream);
    } else {
        FILE *stream;
        stream = fopen(argv[1], "r");

        if (stream == NULL) {
            perror("Couldn't read the input file");
            return ERRC_FILE;
        }

        print_last_n_lines(10, stream);
        fclose(stream);
    }

    return 0;
}