//======== Copyright (c) 2017, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     Test Driven Development - priority queue code
//
// $NoKeywords: $ivs_project_1 $tdd_code.h
// $Author:     Ondřej Ondryáš <xondry02@stud.fit.vutbr.cz>
// $Date:       $2020-02-23
//============================================================================//
/**
 * @file tdd_code.h
 * @author Ondřej Ondryáš
 *
 * @brief Implementace metod tridy prioritni fronty.
 */

#include "tdd_code.h"

//============================================================================//
// ** ZDE DOPLNTE IMPLEMENTACI **
//
// Zde doplnte implementaci verejneho rozhrani prioritni fronty (Priority Queue)
// 1. Verejne rozhrani fronty specifikovane v: tdd_code.h (sekce "public:")
//    - Konstruktor (PriorityQueue()), Destruktor (~PriorityQueue())
//    - Metody Insert/Remove/Find a GetHead
//    - Pripadne vase metody definovane v tdd_code.h (sekce "protected:")
//
// Cilem je dosahnout plne funkcni implementace prioritni fronty implementovane
// pomoci tzv. "double-linked list", ktera bude splnovat dodane testy 
// (tdd_tests.cpp).
//============================================================================//

PriorityQueue::PriorityQueue() {
}

PriorityQueue::~PriorityQueue() {
    Element_t *current;
    Element_t *next = head;

    while (next) {
        current = next;
        next = current->pNext;
        delete current;
    }

    head = nullptr;
}

void PriorityQueue::Insert(int value) {
    auto *newElement = new Element_t();
    newElement->value = value;

    // The new element will become the head
    if (head == nullptr || head->value > value) {
        newElement->pPrev = nullptr;
        newElement->pNext = head;

        if (head != nullptr) {
            head->pPrev = newElement;
        }

        head = newElement;
        return;
    }

    Element_t *i = head;
    while (i->pNext != nullptr && i->pNext->value < value) {
        i = i->pNext;
    } // i now points to the predecessor of the item that is being inserted

    newElement->pNext = i->pNext;
    newElement->pPrev = i;

    i->pNext = newElement;
    if (newElement->pNext != nullptr) {
        newElement->pNext->pPrev = newElement;
    }
}

bool PriorityQueue::Remove(int value) {
    Element_t *element = Find(value);
    if (element == nullptr) return false;

    if (element == head) {
        head = element->pNext;
    }

    if(element->pNext != nullptr) {
        element->pNext->pPrev = element->pPrev;
    }

    if(element->pPrev != nullptr) {
        element->pPrev->pNext = element->pNext;
    }

    delete element;
    return true;
}

PriorityQueue::Element_t *PriorityQueue::Find(int value) {
    if (head == nullptr) return nullptr;

    Element_t *i = head;
    while (i->value != value) {
        i = i->pNext;
        if (i == nullptr) return nullptr;
    }

    return i;
}

PriorityQueue::Element_t *PriorityQueue::GetHead() {
    return head;
}

/*** Konec souboru tdd_code.cpp ***/
