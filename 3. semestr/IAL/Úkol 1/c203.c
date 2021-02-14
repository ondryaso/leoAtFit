/* ******************************* c203.c *********************************** */
/*  Řešení příkladu C203, 1. domácí úloha z IAL                               */
/*  Datum: 5. 10. 2020                                                        */
/*  Autor implementace: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz), FIT      */
/*                                                                            */
/*  Předmět: Algoritmy (IAL) - FIT VUT v Brně                                 */
/*  Úkol: c203 - Fronta znaků v poli                                          */
/*  Referenční implementace: Petr Přikryl, 1994                               */
/*  Přepis do jazyka C: Václav Topinka, září 2005                             */
/*  Úpravy: Kamil Jeřábek, září 2020                                          */
/* ************************************************************************** */
/*
** Implementujte frontu znaků v poli. Přesnou definici typů naleznete
** v hlavičkovém souboru c203.h (ADT fronta je reprezentována strukturou tQueue,
** která obsahuje pole 'arr' pro uložení hodnot ve frontě a indexy f_index
** a b_index. Všechny implementované funkce musí předpokládat velikost pole
** QUEUE_SIZE, i když ve skutečnosti jsou rozměry statického pole definovány
** MAX_QUEUE. Hodnota QUEUE_SIZE slouží k simulaci fronty v různě velkém poli
** a nastavuje se v testovacím skriptu c203-test.c před testováním
** implementovaných funkcí. Hodnota QUEUE_SIZE může nabývat hodnot v rozsahu
** 1 až MAX_QUEUE.
**
** Index f_index ukazuje vždy na první prvek ve frontě. Index b_index
** ukazuje na první volný prvek ve frontě. Pokud je fronta prázdná, ukazují
** oba indexy na stejné místo. Po inicializaci ukazují na první prvek pole,
** obsahují tedy hodnotu 0. Z uvedených pravidel vyplývá, že v poli je vždy
** minimálně jeden prvek nevyužitý.
**
** Při libovolné operaci se žádný z indexů (f_index i b_index) nesnižuje
** vyjma případu, kdy index přesáhne hranici QUEUE_SIZE. V tom případě
** se "posunuje" znovu na začátek pole. Za tímto účelem budete deklarovat
** pomocnou funkci NextIndex, která pro kruhový pohyb přes indexy pole
** využívá operaci "modulo".
**
** Implementujte následující funkce:
**
**    queueInit ..... inicializace fronty
**    nextIndex ..... pomocná funkce - viz popis výše
**    queueEmpty .... test na prázdnost fronty
**    queueFull ..... test, zda je fronta zaplněna (vyčerpána kapacita)
**    queueFront .... přečte hodnoty prvního prvku z fronty
**    queueRemove ... odstraní první prvek fronty
**    queueGet ...... přečte a odstraní první prvek fronty
**    queueUp ....... zařazení prvku na konec fronty
**/

#include "c203.h"

void queueError(int error_code) {
/*
** Vytiskne upozornění na to, že došlo k chybě.
** Tato funkce bude volána z některých dále implementovaných operací.
**
** TUTO FUNKCI, PROSÍME, NEUPRAVUJTE!
*/
    static const char *QERR_STRINGS[
            MAX_QERR + 1] = {"Unknown error", "Queue error: UP", "Queue error: FRONT", "Queue error: REMOVE",
                             "Queue error: GET", "Queue error: INIT"};
    if (error_code <= 0 || error_code > MAX_QERR)
        error_code = 0;
    printf("%s\n", QERR_STRINGS[error_code]);
    err_flag = 1;
}

void queueInit(tQueue *q) {
/*
** Inicializujte frontu následujícím způsobem:
** - všechny hodnoty v poli q->arr nastavte na '*',
** - index na začátek fronty nastavte na 0,
** - index prvního volného místa nastavte také na 0.
**
** V případě, že funkce dostane jako parametr q == NULL, volejte funkci
** queueError(QERR_INIT).
*/
    if (q == NULL) {
        queueError(QERR_INIT);
        return;
    }

    for (int i = 0; i < QUEUE_SIZE; i++) q->arr[i] = '*';
    q->b_index = q->f_index = 0;
}

int nextIndex(int index) {
/*
** Pomocná funkce, která vrací index následujícího prvku v poli.
** Funkci implementujte jako jediný prikaz využívající operace '%'.
** Funkci nextIndex budete využívat v dalších implementovaných funkcích.
*/
    return (index + 1) % (QUEUE_SIZE);
}

int queueEmpty(const tQueue *q) {
/*
** Vrací nenulovou hodnotu, pokud je frona prázdná, jinak vrací hodnotu 0. 
** Funkci je vhodné implementovat jedním příkazem return.
*/
    return q->f_index == q->b_index;
}

int queueFull(const tQueue *q) {
/*
** Vrací nenulovou hodnotu, je-li fronta plná, jinak vrací hodnotu 0. 
** Funkci je vhodné implementovat jedním příkazem return
** s využitím pomocné funkce nextIndex.
*/
    return nextIndex(q->b_index) == q->f_index;
}

void queueFront(const tQueue *q, char *c) {
/*
** Prostřednictvím parametru c vrátí znak ze začátku fronty q.
** Pokud je fronta prázdná, ošetřete to voláním funkce queueError(QERR_FRONT).
** Volání této funkce při prázdné frontě je vždy nutné považovat za nekorektní.
** Bývá to totiž důsledek špatného návrhu algoritmu, ve kterém je fronta
** použita. O takové situaci se proto musí programátor-vývojář dozvědět.
** V opačném případě je ladění programů obtížnější!
**
** Při implementaci využijte dříve definované funkce queueEmpty.
*/
    if (q == NULL || queueEmpty(q)) {
        queueError(QERR_FRONT);
        return;
    }

    *c = q->arr[q->f_index];
}

void queueRemove(tQueue *q) {
/*
** Odstraní znak ze začátku fronty q. Pokud je fronta prázdná, ošetřete
** vzniklou chybu voláním funkce queueError(QERR_REMOVE).
** Hodnotu na uvolněné pozici ve frontě nijak neošetřujte (nepřepisujte).
** Při implementaci využijte dříve definované funkce queueEmpty a nextIndex.
*/
    if (q == NULL || queueEmpty(q)) {
        queueError(QERR_REMOVE);
        return;
    }

    q->f_index = nextIndex(q->f_index);
}

void queueGet(tQueue *q, char *c) {
/*
** Odstraní znak ze začátku fronty a vrátí ho prostřednictvím parametru c.
** Pokud je fronta prázdná, ošetřete to voláním funkce queueError(QERR_GET).
**
** Při implementaci využijte dříve definovaných funkcí queueEmpty,
** queueFront a queueRemove.
*/
    if (q == NULL || queueEmpty(q)) {
        queueError(QERR_GET);
        return;
    }

    queueFront(q, c);
    queueRemove(q);
}

void queueUp(tQueue *q, char c) {
/*
** Vloží znak c do fronty. Pokud je fronta plná, ošetřete chybu voláním
** funkce queueError(QERR_UP). Vkládání do plné fronty se považuje za
** nekorektní operaci. Situace by mohla být řešena i tak, že by operace
** neprováděla nic, ale v případě použití takto definované abstrakce by se
** obtížně odhalovaly chyby v algoritmech, které by abstrakci využívaly.
**
** Při implementaci využijte dříve definovaných funkcí queueFull a nextIndex.
*/
    if (q == NULL || queueFull(q)) {
        queueError(QERR_UP);
        return;
    }

    q->arr[q->b_index] = c;
    q->b_index = nextIndex(q->b_index);
}
/* Konec příkladu c203.c */
