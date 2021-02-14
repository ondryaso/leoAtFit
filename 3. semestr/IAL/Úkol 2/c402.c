/* c402.c
 * Řešení příkladu C402, 2. domácí úloha z IAL
 * Datum: 8. 11. 2020
 * Autor implementace: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz), FIT
 *
 * Téma: Nerekurzivní implementace operací nad BVS
 * Kostra a její modifikace: Petr Přikryl, Martin Tuček, Bohuslav Křena,
 *                           Karel Masařík, Radek Hranický
**/

#include "c402.h"

int solved;

/*
 * Pomocná funkce, kterou budete volat při průchodech stromem pro zpracování
 * uzlu určeného ukazatelem Ptr.
 */
void BTWorkOut(tBTNodePtr Ptr) {
    if (Ptr == NULL)
        printf("Chyba: Funkce BTWorkOut byla volána s NULL argumentem!\n");
    else
        printf("Výpis hodnoty daného uzlu> %d\n", Ptr->Cont);
}

/* -------------------------------------------------------------------------- */
/*
 * Funkce pro zásobník hodnot typu tBTNodePtr.
 */

/*
 * Inicializace zásobníku.
 */
void SInitP(tStackP *S) {
    S->top = 0;
}

/*
 * Vloží hodnotu na vrchol zásobníku.
 */
void SPushP(tStackP *S, tBTNodePtr ptr) {
    /* Při implementaci v poli může dojít k přetečení zásobníku. */
    if (S->top == MAXSTACK)
        printf("Chyba: Došlo k přetečení zásobníku s ukazateli!\n");
    else {
        S->top++;
        S->a[S->top] = ptr;
    }
}

/*
 * Odstraní prvek z vrcholu zásobníku a současně vrátí jeho hodnotu.
 */
tBTNodePtr STopPopP(tStackP *S) {
    /* Operace nad prázdným zásobníkem způsobí chybu. */
    if (S->top == 0) {
        printf("Chyba: Došlo k podtečení zásobníku s ukazateli!\n");
        return (NULL);
    } else {
        return (S->a[S->top--]);
    }
}

/*
 * Je-li zásobník prázdný, vrátí hodnotu true.
 */
bool SEmptyP(tStackP *S) {
    return (S->top == 0);
}

/* -------------------------------------------------------------------------- */
/*
 * Funkce pro zásobník hotnot typu bool.
*/

/*
 * Inicializace zásobníku.
 */
void SInitB(tStackB *S) {
    S->top = 0;
}

/*
 * Vloží hodnotu na vrchol zásobníku.
 */
void SPushB(tStackB *S, bool val) {
    /* Při implementaci v poli může dojít k přetečení zásobníku. */
    if (S->top == MAXSTACK)
        printf("Chyba: Došlo k přetečení zásobníku pro boolean!\n");
    else {
        S->top++;
        S->a[S->top] = val;
    }
}

/*
 * Odstraní prvek z vrcholu zásobníku a současně vrátí jeho hodnotu.
 */
bool STopPopB(tStackB *S) {
    /* Operace nad prázdným zásobníkem způsobí chybu. */
    if (S->top == 0) {
        printf("Chyba: Došlo k podtečení zásobníku pro boolean!\n");
        return (NULL);
    } else {
        return (S->a[S->top--]);
    }
}

/*
 * Je-li zásobník prázdný, vrátí hodnotu true.
 */
bool SEmptyB(tStackB *S) {
    return (S->top == 0);
}

/* -------------------------------------------------------------------------- */

/*
 * Provede inicializaci binárního vyhledávacího stromu.
 *
 * Inicializaci smí programátor volat pouze před prvním použitím binárního
 * stromu, protože neuvolňuje uzly neprázdného stromu (a ani to dělat nemůže,
 * protože před inicializací jsou hodnoty nedefinované, tedy libovolné).
 * Ke zrušení binárního stromu slouží procedura BTDisposeTree.
 */
void BTInit(tBTNodePtr *RootPtr) {
    (*RootPtr) = NULL;
}

/*
 * Vloží do stromu nový uzel s hodnotou Content.
 *
 * Z pohledu vkládání chápejte vytvářený strom jako binární vyhledávací strom,
 * kde uzly s hodnotou menší než má otec leží v levém podstromu a uzly větší
 * leží vpravo. Pokud vkládaný uzel již existuje, neprovádí se nic (daná hodnota
 * se ve stromu může vyskytnout nejvýše jednou). Pokud se vytváří nový uzel,
 * vzniká vždy jako list stromu.
 */
void BTInsert(tBTNodePtr *RootPtr, int Content) {
    if (RootPtr == NULL) return;
    tBTNodePtr root = *RootPtr;

    while (root != NULL) {
        if (root->Cont == Content) {
            return;
        } else if (root->Cont > Content) {
            RootPtr = &root->LPtr;
        } else {
            RootPtr = &root->RPtr;
        }
        root = *RootPtr;
    }

    root = calloc(1, sizeof(struct tBTNode));
    root->Cont = Content;
    *RootPtr = root;
}

/*                                  PREORDER                                  */

/*
 * Jde po levé větvi podstromu, dokud nenarazí na jeho nejlevější uzel.
 *
 * Při průchodu Preorder navštívené uzly zpracujeme voláním funkce BTWorkOut()
 * a ukazatele na ně is uložíme do zásobníku.
 */
void Leftmost_Preorder(tBTNodePtr ptr, tStackP *Stack) {
    while (ptr != NULL) {
        SPushP(Stack, ptr);
        BTWorkOut(ptr);
        ptr = ptr->LPtr;
    }
}

/*
 * Průchod stromem typu preorder implementovaný nerekurzivně s využitím funkce
 * Leftmost_Preorder a zásobníku ukazatelů. Zpracování jednoho uzlu stromu
 * realizováno jako volání funkce BTWorkOut().
 */
void BTPreorder(tBTNodePtr RootPtr) {
    if (RootPtr == NULL) return;

    tStackP stack;
    SInitP(&stack);
    Leftmost_Preorder(RootPtr, &stack);
    while (!SEmptyP(&stack)) {
        RootPtr = STopPopP(&stack);
        Leftmost_Preorder(RootPtr->RPtr, &stack);
    }
}


/*                                  INORDER                                   */

/*   ----------------
 * Jde po levě větvi podstromu, dokud nenarazí na jeho nejlevější uzel.
 *
 * Při průchodu Inorder ukládáme ukazatele na všechny navštívené uzly do
 * zásobníku.
 */
void Leftmost_Inorder(tBTNodePtr ptr, tStackP *Stack) {
    while (ptr != NULL) {
        SPushP(Stack, ptr);
        ptr = ptr->LPtr;
    }
}

/*
 * Průchod stromem typu Inorder implementovaný nerekurzivně s využitím funkce
 * Leftmost_Inorder a zásobníku ukazatelů. Zpracování jednoho uzlu stromu
 * realizováno jako volání funkce BTWorkOut().
**/
void BTInorder(tBTNodePtr RootPtr) {
    if (RootPtr == NULL) return;

    tStackP stack;
    SInitP(&stack);
    Leftmost_Inorder(RootPtr, &stack);
    while (!SEmptyP(&stack)) {
        RootPtr = STopPopP(&stack);
        BTWorkOut(RootPtr);
        Leftmost_Inorder(RootPtr->RPtr, &stack);
    }
}

/*                                 POSTORDER                                  */

/*
 * Jde po levé větvi podstromu, dokud nenarazí na jeho nejlevější uzel.
 *
 * Při průchodu Postorder ukládáme ukazatele na navštívené uzly do zásobníku
 * a současně do zásobníku bool hodnot ukládáme informaci, zda byl uzel
 * navštíven poprvé, a že se tedy ještě nemá zpracovávat.
**/
void Leftmost_Postorder(tBTNodePtr ptr, tStackP *StackP, tStackB *StackB) {
    while (ptr != NULL) {
        SPushP(StackP, ptr);
        SPushB(StackB, true);
        ptr = ptr->LPtr;
    }
}

/*
 * Průchod stromem typu Postorder implementovaný nerekurzivně s využitím funkce
 * Leftmost_Postorder, zásobníku ukazatelů a zásobníku hotdnot typu bool.
 * Zpracování jednoho uzlu stromu realizováno jako volání funkce BTWorkOut().
 */
void BTPostorder(tBTNodePtr RootPtr) {
    if (RootPtr == NULL) return;

    tStackP sP;
    tStackB sB;
    SInitP(&sP);
    SInitB(&sB);

    Leftmost_Postorder(RootPtr, &sP, &sB);

    while (!SEmptyP(&sP)) {
        RootPtr = STopPopP(&sP);
        bool isComingFromLeft = STopPopB(&sB);

        if (isComingFromLeft) {
            SPushB(&sB, false);
            SPushP(&sP, RootPtr);
            Leftmost_Postorder(RootPtr->RPtr, &sP, &sB);
        } else {
            BTWorkOut(RootPtr);
        }
    }
}

/*
 * Zruší všechny uzly stromu a korektně uvolní jimi zabranou paměť.
 */
void BTDisposeTree(tBTNodePtr *RootPtr) {

    if (RootPtr == NULL) return;
    tStackP stack;
    SInitP(&stack);
    tBTNodePtr root = *RootPtr;
    do {
        if (root != NULL) {
            if (root->RPtr != NULL) {
                SPushP(&stack, root->RPtr);
            }

            tBTNodePtr toDelete = root;
            root = root->LPtr;

            free(toDelete);
        } else {
            if (!SEmptyP(&stack)) {
                root = STopPopP(&stack);
            }
        }
    } while (root != NULL || !SEmptyP(&stack));

    *RootPtr = NULL;
}

/* konec c402.c */

