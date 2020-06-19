// error.c
// Řešení IJC-DU1
// příklad B)
// Datum: 13. 3. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT

#include "error.h"

// I am tempted to put a \n to the end, it would make sense,
// but I'm sticking to the assignment (and appending the \n
// to all the error message anyway.)
void vwarning_msg(const char *fmt, va_list args) {
    fprintf(stderr, "CHYBA: ");
    vfprintf(stderr, fmt, args);
}

void warning_msg(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vwarning_msg(fmt, args);
    va_end(args);
}

void error_exit(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vwarning_msg(fmt, args);
    va_end(args);

    exit(1);
}