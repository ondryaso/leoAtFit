/* c206.c
** Řešení příkladu C206, 1. domácí úloha z IAL
** Datum: 5. 10. 2020
** Autor implementace: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz), FIT
**
** Téma: Dvousměrně vázaný lineární seznam
**
**                   Návrh a referenční implementace: Bohuslav Křena, říjen 2001
**                            Přepracované do jazyka C: Martin Tuček, říjen 2004
**                                            Úpravy: Kamil Jeřábek, září 2020
**
** Implementujte abstraktní datový typ dvousměrně vázaný lineární seznam.
** Užitečným obsahem prvku seznamu je hodnota typu int.
** Seznam bude jako datová abstrakce reprezentován proměnnou
** typu tDLList (DL znamená Double-Linked a slouží pro odlišení
** jmen konstant, typů a funkcí od jmen u jednosměrně vázaného lineárního
** seznamu). Definici konstant a typů naleznete v hlavičkovém souboru c206.h.
**
** Vaším úkolem je implementovat následující operace, které spolu
** s výše uvedenou datovou částí abstrakce tvoří abstraktní datový typ
** obousměrně vázaný lineární seznam:
**
**      DLInitList ...... inicializace seznamu před prvním použitím,
**      DLDisposeList ... zrušení všech prvků seznamu,
**      DLInsertFirst ... vložení prvku na začátek seznamu,
**      DLInsertLast .... vložení prvku na konec seznamu,
**      DLFirst ......... nastavení aktivity na první prvek,
**      DLLast .......... nastavení aktivity na poslední prvek,
**      DLCopyFirst ..... vrací hodnotu prvního prvku,
**      DLCopyLast ...... vrací hodnotu posledního prvku,
**      DLDeleteFirst ... zruší první prvek seznamu,
**      DLDeleteLast .... zruší poslední prvek seznamu,
**      DLPostDelete .... ruší prvek za aktivním prvkem,
**      DLPreDelete ..... ruší prvek před aktivním prvkem,
**      DLPostInsert .... vloží nový prvek za aktivní prvek seznamu,
**      DLPreInsert ..... vloží nový prvek před aktivní prvek seznamu,
**      DLCopy .......... vrací hodnotu aktivního prvku,
**      DLActualize ..... přepíše obsah aktivního prvku novou hodnotou,
**      DLPred .......... posune aktivitu na předchozí prvek seznamu,
**      DLSucc .......... posune aktivitu na další prvek seznamu,
**      DLActive ........ zjišťuje aktivitu seznamu.
**
** Při implementaci jednotlivých funkcí nevolejte žádnou z funkcí
** implementovaných v rámci tohoto příkladu, není-li u funkce
** explicitně uvedeno něco jiného.
**
** Nemusíte ošetřovat situaci, kdy místo legálního ukazatele na seznam 
** předá někdo jako parametr hodnotu NULL.
**/

#include "c206.h"

int solved;
int errflg;

void DLError() {
/*
** Vytiskne upozornění na to, že došlo k chybě.
** Tato funkce bude volána z některých dále implementovaných operací.
**/
    printf("*ERROR* The program has performed an illegal operation.\n");
    errflg = TRUE;             /* globální proměnná -- příznak ošetření chyby */
    return;
}

void DLInitList(tDLList *L) {
/*
** Provede inicializaci seznamu L před jeho prvním použitím (tzn. žádná
** z následujících funkcí nebude volána nad neinicializovaným seznamem).
** Tato inicializace se nikdy nebude provádět nad již inicializovaným
** seznamem, a proto tuto možnost neošetřujte. Vždy předpokládejte,
** že neinicializované proměnné mají nedefinovanou hodnotu.
**/
    L->Act = L->First = L->Last = NULL;
}

void DLDisposeList(tDLList *L) {
/*
** Zruší všechny prvky seznamu L a uvede seznam do stavu, v jakém
** se nacházel po inicializaci. Rušené prvky seznamu budou korektně
** uvolněny voláním operace free. 
**/
    while (L->First != NULL) {
        tDLElemPtr item = L->First->rptr;
        free(L->First);
        L->First = item;
    }

    L->Act = L->First = L->Last = NULL;
}

void DLInsertFirst(tDLList *L, int val) {
/*
** Vloží nový prvek na začátek seznamu L.
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
    tDLElemPtr item = malloc(sizeof(struct tDLElem));
    if (item == NULL) {
        DLError();
        return;
    }

    item->lptr = NULL;
    item->rptr = L->First;
    item->data = val;

    if (L->First != NULL) {
        L->First->lptr = item;
    } else {
        L->Last = item;
    }

    L->First = item;
}

void DLInsertLast(tDLList *L, int val) {
/*
** Vloží nový prvek na konec seznamu L (symetrická operace k DLInsertFirst).
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
    tDLElemPtr item = malloc(sizeof(struct tDLElem));
    if (item == NULL) {
        DLError();
        return;
    }

    item->lptr = L->Last;
    item->rptr = NULL;
    item->data = val;

    if (L->Last != NULL) {
        L->Last->rptr = item;
    } else {
        L->First = item;
    }

    L->Last = item;
}

void DLFirst(tDLList *L) {
/*
** Nastaví aktivitu na první prvek seznamu L.
** Funkci implementujte jako jediný příkaz (nepočítáme-li return),
** aniž byste testovali, zda je seznam L prázdný.
**/
    L->Act = L->First;
}

void DLLast(tDLList *L) {
/*
** Nastaví aktivitu na poslední prvek seznamu L.
** Funkci implementujte jako jediný příkaz (nepočítáme-li return),
** aniž byste testovali, zda je seznam L prázdný.
**/
    L->Act = L->Last;
}

void DLCopyFirst(tDLList *L, int *val) {
/*
** Prostřednictvím parametru val vrátí hodnotu prvního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/
    if (L->First == NULL) {
        DLError();
        return;
    }

    *val = L->First->data;
}

void DLCopyLast(tDLList *L, int *val) {
/*
** Prostřednictvím parametru val vrátí hodnotu posledního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/
    if (L->First == NULL) {
        DLError();
        return;
    }

    *val = L->Last->data;
}

void DLDeleteFirst(tDLList *L) {
/*
** Zruší první prvek seznamu L. Pokud byl první prvek aktivní, aktivita 
** se ztrácí. Pokud byl seznam L prázdný, nic se neděje.
**/
    if (L->First == NULL) return;
    if (L->Act == L->First) L->Act = NULL;

    tDLElemPtr next = L->First->rptr;

    if (next == NULL) { // First == Last
        L->Last = NULL;
    } else {
        next->lptr = NULL;
    }

    free(L->First);
    L->First = next;
}

void DLDeleteLast(tDLList *L) {
/*
** Zruší poslední prvek seznamu L.
** Pokud byl poslední prvek aktivní, aktivita seznamu se ztrácí.
** Pokud byl seznam L prázdný, nic se neděje.
**/
    if (L->First == NULL) return;
    if (L->Act == L->Last) L->Act = NULL;

    tDLElemPtr prev = L->Last->lptr;

    if (prev == NULL) { // First == Last
        L->First = NULL;
    } else {
        prev->rptr = NULL;
    }

    free(L->Last);
    L->Last = prev;
}

void DLPostDelete(tDLList *L) {
/*
** Zruší prvek seznamu L za aktivním prvkem.
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** posledním prvkem seznamu, nic se neděje.
**/
    if (L->Act == NULL || L->Act == L->Last) return;

    tDLElemPtr next = L->Act->rptr;
    L->Act->rptr = next->rptr; // Relink left-to-right
    if (next->rptr != NULL) { // Relink right-to-left
        next->rptr->lptr = L->Act;
    } else { // The next item was the last one
        L->Last = L->Act;
    }

    free(next);
}

void DLPreDelete(tDLList *L) {
/*
** Zruší prvek před aktivním prvkem seznamu L .
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** prvním prvkem seznamu, nic se neděje.
**/
    if (L->Act == NULL || L->Act == L->First) return;

    tDLElemPtr prev = L->Act->lptr;
    L->Act->lptr = prev->lptr;
    if (prev->lptr != NULL) {
        prev->lptr->rptr = L->Act;
    } else { // The previous item was the First one
        L->First = L->Act;
    }

    free(prev);
}

void DLPostInsert(tDLList *L, int val) {
/*
** Vloží prvek za aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se neděje.
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
    if (L->Act == NULL) return;

    tDLElemPtr new = malloc(sizeof(struct tDLElem));
    if (new == NULL) {
        DLError();
        return;
    }

    new->lptr = L->Act;
    new->rptr = L->Act->rptr;

    if (new->rptr != NULL) {
        new->rptr->lptr = new;
    }

    new->data = val;
    L->Act->rptr = new;
}

void DLPreInsert(tDLList *L, int val) {
/*
** Vloží prvek před aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se neděje.
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
    if (L->Act == NULL) return;

    tDLElemPtr new = malloc(sizeof(struct tDLElem));
    if (new == NULL) {
        DLError();
        return;
    }

    new->rptr = L->Act;
    new->lptr = L->Act->lptr;

    if (new->lptr != NULL) {
        new->lptr->rptr = new;
    }

    new->data = val;
    L->Act->lptr = new;
}

void DLCopy(tDLList *L, int *val) {
/*
** Prostřednictvím parametru val vrátí hodnotu aktivního prvku seznamu L.
** Pokud seznam L není aktivní, volá funkci DLError ().
**/
    if (L->Act == NULL) {
        DLError();
        return;
    }

    *val = L->Act->data;
}

void DLActualize(tDLList *L, int val) {
/*
** Přepíše obsah aktivního prvku seznamu L.
** Pokud seznam L není aktivní, nedělá nic.
**/
    if (L->Act == NULL) return;
    L->Act->data = val;
}

void DLSucc(tDLList *L) {
/*
** Posune aktivitu na následující prvek seznamu L.
** Není-li seznam aktivní, nedělá nic.
** Všimněte si, že při aktivitě na posledním prvku se seznam stane neaktivním.
**/
    if (L->Act == NULL) return;
    L->Act = L->Act->rptr;
}


void DLPred(tDLList *L) {
/*
** Posune aktivitu na předchozí prvek seznamu L.
** Není-li seznam aktivní, nedělá nic.
** Všimněte si, že při aktivitě na prvním prvku se seznam stane neaktivním.
**/
    if (L->Act == NULL) return;
    L->Act = L->Act->lptr;
}

int DLActive(tDLList *L) {
/*
** Je-li seznam L aktivní, vrací nenulovou hodnotu, jinak vrací 0.
** Funkci je vhodné implementovat jedním příkazem return.
**/
    return L->Act != NULL;
}

/* Konec c206.c*/
