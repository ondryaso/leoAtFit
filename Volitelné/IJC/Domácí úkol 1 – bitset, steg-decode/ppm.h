// ppm.h
// Řešení IJC-DU1
// příklad B)
// Datum: 13. 3. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT

#ifndef PPM_H
#define PPM_H

struct ppm {
    unsigned xsize;
    unsigned ysize;
    char data[];
};

struct ppm * ppm_read(const char * filename);
void ppm_free(struct ppm *p);

typedef struct ppm *ppm_t;

#endif