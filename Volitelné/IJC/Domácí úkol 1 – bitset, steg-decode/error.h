// error.h
// Řešení IJC-DU1
// příklad B)
// Datum: 13. 3. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT

#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void vwarning_msg(const char *fmt, va_list arg);
void warning_msg(const char *fmt, ...);
void error_exit(const char *fmt, ...);

#endif
