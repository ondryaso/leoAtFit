/* c401.c
 * Řešení příkladu C401, 2. domácí úloha z IAL
 * Datum: 8. 11. 2020
 * Autor implementace: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz), FIT
**
 * Téma: Rekurzivní implementace operací nad BVS
 * Kostra a její mofifikace: Petr Přikryl, Andrea Němcová, Martin Tuček,
 *                           Bohuslav Křena, Karel Masařík, Radek Hranický
**/

#include "c401.h"

int solved;

/*
 * Funkce provede počáteční inicializaci stromu před jeho prvním použitím.
 */
void BSTInit(tBSTNodePtr *RootPtr) {
    *RootPtr = NULL;
}

/*
 * Funkce vyhledá uzel v BVS s klíčem K.
 *
 * Pokud je takový nalezen, vrací funkce hodnotu TRUE a v proměnné Content se
 * vrací obsah příslušného uzlu. Pokud příslušný uzel není nalezen, vrací funkce
 * hodnotu FALSE a obsah proměnné Content není definován.
 */
int BSTSearch(tBSTNodePtr RootPtr, char K, int *Content) {
    if (RootPtr == NULL) return FALSE;

    if (RootPtr->Key == K) {
        *Content = RootPtr->BSTNodeCont;
        return TRUE;
    } else if (RootPtr->Key > K) {
        // Hledaný klíč je menší než klíč aktuálního, jdeme doleva
        return BSTSearch(RootPtr->LPtr, K, Content);
    } else {
        // Hledaný klíč je větší než klíč aktuálního, jdeme doprava
        return BSTSearch(RootPtr->RPtr, K, Content);
    }
}

/*
 * Vloží do stromu RootPtr hodnotu Content s klíčem K.
 *
 * Pokud již uzel se zadaným klíčem ve stromu existuje, bude obsah uzlu
 * s klíčem K nahrazen novou hodnotou. Pokud bude do stromu vložen nový
 * uzel, bude vložen vždy jako list stromu.
 *
 * Rekurzivní implementace je méně efektivní, protože se při každém
 * rekurzivním zanoření ukládá na zásobník obsah uzlu (zde integer).
 * Nerekurzivní varianta by v tomto případě byla efektivnější jak z hlediska
 * rychlosti, tak z hlediska paměťových nároků.
 */
void BSTInsert(tBSTNodePtr *RootPtr, char K, int Content) {
    if (RootPtr == NULL) return;

    tBSTNodePtr currentRoot = *RootPtr;
    if (currentRoot == NULL) {
        currentRoot = malloc(sizeof(struct tBSTNode));
        currentRoot->Key = K;
        currentRoot->BSTNodeCont = Content;
        currentRoot->LPtr = currentRoot->RPtr = NULL;
        *RootPtr = currentRoot;
        return;
    }

    if (currentRoot->Key == K) {
        currentRoot->BSTNodeCont = Content;
    } else if (currentRoot->Key > K) {
        // Vkládaný klíč je menší než aktuální, budeme vkládat někam do levé větve
        BSTInsert(&currentRoot->LPtr, K, Content);
    } else {
        BSTInsert(&currentRoot->RPtr, K, Content);
    }
}

/*
 * Pomocná funkce pro vyhledání, přesun a uvolnění nejpravějšího uzlu.
 *
 * Ukazatel PtrReplaced ukazuje na uzel, do kterého bude přesunuta hodnota
 * nejpravějšího uzlu v podstromu, který je určen ukazatelem RootPtr.
 * Předpokládá se, že hodnota ukazatele RootPtr nebude NULL (zajistěte to
 * testováním před volání této funkce).
 */
void ReplaceByRightmost(tBSTNodePtr PtrReplaced, tBSTNodePtr *RootPtr) {

    tBSTNodePtr root = *RootPtr;

    if (root->RPtr != NULL) {
        ReplaceByRightmost(PtrReplaced, &root->RPtr);
    } else {
        PtrReplaced->Key = root->Key;
        PtrReplaced->BSTNodeCont = root->BSTNodeCont;
        *RootPtr = root->LPtr;
        free(root);
    }
}

/*
 * Zruší uzel stromu, který obsahuje klíč K.
 *
 * Pokud uzel se zadaným klíčem neexistuje, nedělá funkce nic.
 * Pokud má rušený uzel jen jeden podstrom, pak jej zdědí otec rušeného uzlu.
 * Pokud má rušený uzel oba podstromy, pak je rušený uzel nahrazen nejpravějším
 * uzlem levého podstromu.
 */
void BSTDelete(tBSTNodePtr *RootPtr, char K) {
    if (RootPtr == NULL) return;
    tBSTNodePtr root = *RootPtr;

    if (root == NULL) return; // Klíč s danou hodnotou nenalezen

    if (root->Key == K) {
        if (root->LPtr == NULL && root->RPtr == NULL) {
            // Nemá podstrom
            free(root);
            *RootPtr = NULL;
        } else if (root->LPtr != NULL && root->RPtr != NULL) {
            // Má oba podstromy
            ReplaceByRightmost(root, &root->LPtr);
        } else {
            *RootPtr = root->LPtr != NULL ? root->LPtr : root->RPtr;
            free(root);
        }
    } else if (root->Key > K) {
        BSTDelete(&root->LPtr, K);
    } else {
        BSTDelete(&root->RPtr, K);
    }
}

/*
 * Zruší celý binární vyhledávací strom a korektně uvolní paměť.
 *
 * Po zrušení se bude BVS nacházet ve stejném stavu, jako se nacházel po
 * inicializaci.
 */
void BSTDispose(tBSTNodePtr *RootPtr) {

    if (RootPtr == NULL) return;
    tBSTNodePtr root = *RootPtr;
    if (root == NULL) return;

    BSTDispose(&root->LPtr);
    BSTDispose(&root->RPtr);
    free(root);
    *RootPtr = NULL;
}

/* konec c401.c */

