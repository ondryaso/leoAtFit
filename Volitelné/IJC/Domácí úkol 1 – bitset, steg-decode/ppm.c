// ppm.c
// Řešení IJC-DU1
// příklad B)
// Datum: 13. 3. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT

#include "ppm.h"
#include "error.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

struct ppm *ppm_read(const char *filename) {
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL) {
        error_exit("Nebylo možné otevřít soubor \"%s\" (%s).\n", filename, strerror(errno));
    }

    unsigned int width, height;
    int res = fscanf(fp, "P6 %u %u 255", &width, &height);

    if (res != 2) {
        int err = ferror(fp);
        fclose(fp);

        if (err) {
            error_exit("Chyba při čtení souboru (%s).\n", strerror(err));
        } else {
            error_exit("Neplatný formát souboru (neplatná hlavička).\n");
        }
    }

    res = fgetc(fp);
    if (!isspace(res)) {
        fclose(fp);
        error_exit("Neplatný formát souboru (neplatný oddělovač dat).\n");
    }

    unsigned long dataSize = width * height * 3 * sizeof(char);
    ppm_t img = malloc(sizeof(ppm_t) + dataSize);

    img->xsize = width;
    img->ysize = height;
    size_t readBytes = fread(img->data, sizeof(char), dataSize, fp);

    // It's necessary to call fclose() before every error_exit(), otherwise,
    // the file handle would never get closed (it would, eventually, but not directly).
    if (readBytes != dataSize) {
        int err = ferror(fp);
        free(img);

        if (err) {
            fclose(fp);
            error_exit("Chyba při čtení souboru (%s).\n", strerror(err));
        } else {
            if (feof(fp)) {
                fclose(fp);
                error_exit("Neplatný formát souboru (soubor neobsahuje určené množství dat).\n");
            } else {
                fclose(fp);
                error_exit("Neplatný formát souboru (neznámá chyba).\n");
            }
        }
    }

    if(fgetc(fp) != EOF) {
        free(img);
        fclose(fp);
        error_exit("Neplatný formát souboru (neočekávaná data).\n");
    }

    fclose(fp);
    return img;
}

void ppm_free(struct ppm *p) {
    if (p != NULL) {
        free(p);
    }
}