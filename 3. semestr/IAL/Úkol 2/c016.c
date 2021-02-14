/* c016.c
 * Řešení příkladu C016, 2. domácí úloha z IAL
 * Datum: 8. 11. 2020
 * Autor implementace: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz), FIT
**
 * Téma: Tabulka s rozptýlenými položkami
 * Kostra a její modifikace: Petr Přikryl, Václav Topinka, Karel Masařík,
 *                           Radek Hranický
**/

#include "c016.h"

int HTSIZE = MAX_HTSIZE;
int solved;

/*
 * Rozptylovací funkce - jejím úkolem je zpracovat zadaný klíč a přidělit
 * mu index v rozmezí 0..HTSize-1. V ideálním případě by mělo dojít
 * k rovnoměrnému rozptýlení těchto klíčů po celé tabulce.
 * (Funkce nebyla volena s ohledem na maximální kvalitu výsledku).
*/
int hashCode(tKey key) {
    int retval = 1;
    int keylen = strlen(key);
    for (int i = 0; i < keylen; i++)
        retval += key[i];
    return (retval % HTSIZE);
}

/*
 * Inicializace tabulky s explicitně zřetězenými synonymy.  Tato procedura
 * se volá pouze před prvním použitím tabulky.
*/
void htInit(tHTable *ptrht) {
    if (ptrht == NULL) return;
    memset(ptrht, 0, sizeof(tHTItem *) * HTSIZE);
}

/* TRP s explicitně zřetězenými synonymy.
 * Vyhledání prvku v TRP ptrht podle zadaného klíče key.
 * Pokud je daný prvek nalezen, vrací se ukazatel na daný prvek.
 * Pokud prvek nalezen není, vrací se hodnota NULL.
 */
tHTItem *htSearch(tHTable *ptrht, tKey key) {
    if (ptrht == NULL) return NULL;

    int hash = hashCode(key);
    tHTItem *it = (*ptrht)[hash];

    while (it != NULL && strcmp(it->key, key) != 0) {
        it = it->ptrnext;
    }

    if (it == NULL) return NULL;
    return it;
}

/*
 * Tato funkce vkládá do tabulky ptrht položku s klíčem key a s daty
 * data. Protože jde o vyhledávací tabulku, nemůže být prvek se stejným
 * klíčem uložen v tabulce více než jedenkrát. Pokud se vkládá prvek,
 * jehož klíč se již v tabulce nachází, aktualizujte jeho datovou část.
 */
void htInsert(tHTable *ptrht, tKey key, tData data) {
    if (ptrht == NULL) return;

    tHTItem *it = htSearch(ptrht, key);

    if (it != NULL) {
        it->data = data;
    } else {
        int hash = hashCode(key);

        tHTItem *new = malloc(sizeof(struct tHTItem));

        if (new == NULL) {
            return; // ERROR!
        }

        new->key = key;
        new->data = data;
        new->ptrnext = (*ptrht)[hash];

        (*ptrht)[hash] = new;
    }
}

/*
 * Tato funkce zjišťuje hodnotu datové části položky zadané klíčem.
 * Pokud je položka nalezena, vrací funkce ukazatel na položku
 * Pokud položka nalezena nebyla, vrací se funkční hodnota NULL
 */
tData *htRead(tHTable *ptrht, tKey key) {
    tHTItem *it = htSearch(ptrht, key);
    if (it != NULL) return &it->data;
    return NULL;
}

/*
 * Tato procedura vyjme položku s klíčem key z tabulky ptrht.
 * Pokud položka s uvedeným klíčem neexistuje, nestane se nic.
 */
void htDelete(tHTable *ptrht, tKey key) {
    if (ptrht == NULL) return;

    int hash = hashCode(key);
    tHTItem *it = (*ptrht)[hash];
    tHTItem *prev = NULL;

    while (it != NULL && strcmp(it->key, key) != 0) {
        prev = it;
        it = it->ptrnext;
    }

    if (it == NULL) return;
    if (prev == NULL) {
        // Hledaná položka je prvním ze synonym
        (*ptrht)[hash] = it->ptrnext;
    } else {
        // Hledaná položka má předchůdce, převázat seznam
        prev->ptrnext = it->ptrnext;
    }

    free(it);
}

/*
 * Tato procedura zruší všechny položky tabulky, korektně uvolní prostor,
 * který tyto položky zabíraly, a uvede tabulku do počátečního stavu.
 */
void htClearAll(tHTable *ptrht) {
    if (ptrht == NULL) return;

    for (int i = 0; i < HTSIZE; i++) {
        tHTItem *it = (*ptrht)[i];
        // if (it == NULL) continue; // Není třeba, pokryto neprovedením následujícího cyklu

        while (it != NULL) {
            tHTItem *toDelete = it;
            it = it->ptrnext;
            free(toDelete);
        }

        (*ptrht)[i] = NULL;
    }
}
