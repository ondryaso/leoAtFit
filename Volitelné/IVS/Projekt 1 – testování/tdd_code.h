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
 * @brief Definice rozhrani prioritni fronty.
 */

#pragma once

#ifndef TDD_CODE_H_
#define TDD_CODE_H_

/**
 * @brief The PriorityQueue class
 * Prioritni fronta (polozky vzdy serazeny od min po max) implementovana pomoci
 * tzv. double-linked listu (kazda polozka ma odkaz na predchazejici a nasledujici polozku).
 * Dale ma kazda polozka hodnotu typu "int", pricemz fronta muze obsahovat vice
 * polozek se stejnou hodnotou.
 */
class PriorityQueue
{
public:
    //============================================================================//
    // ** METODY, STRUKTURY ATD. V TETO SEKCI *NEUPRAVUJTE* **
    //
    // Metody, struktury atd. totiz tvori verejne "public" rozhrani prioritni
    // fronty.
    //

    /**
     * @brief PriorityQueue
     * Konstruktor, vytvori prazdnou frontu.
     */
    PriorityQueue();

    /**
     * @brief ~PriorityQueue
     * Destruktor, odstrani vsechny polozky i frontu samotnou.
     */
    ~PriorityQueue();

    /**
     * @brief The Element_t struct
     * Struktura polozky ve fronte.
     */
    struct Element_t {
        Element_t *pNext;   ///< Ukazatel na nasledujici prvek ve fronte.
        Element_t *pPrev;   ///< Ukazatel na predchazejici prvek ve fronte.

        int value;          ///< Hodnota teto polozky ve fronte.
    };

    /**
     * @brief Insert
     * Zaradi novou polozku s hodnotou "value" do fronty na patricne misto (tak
     * aby bylo zachovano poradi min->max). Pokud polozka s danou hodnotou jiz
     * existuji zaradi novou polozku pred/za jiz existujici.
     * @param value Hodnota nove polozky.
     */
    void Insert(int value);

    /**
     * @brief Remove
     * Odstrani polozku s hodnotou "value" z fronty a vrati "true", pokud polozka
     * neni nalezena vrati "false". Pokud se ve fronte nachazi vice polozek se
     * stejnou hodnotou "value", pak odstrani libovolnou z nich.
     * @param value Hodnota polozky, ktera ma byt odstranena.
     * @return Vrati true, pokud byla polozka nalezena a odstranena, jinak vraci false.
     */
    bool Remove(int value);

    /**
     * @brief Find
     * Nalezne libovolnou polozku s hodnotou "value" a vrati ukazatel na tuto polozku,
     * nebo NULL pokud takova neexistuje.
     * @param value Hodnota hledane polozky.
     * @return Vrati ukazatel na polozku s hodnotou "value", nebo NULL pokud takova neexistuje.
     */
    Element_t *Find(int value);

    /**
     * @brief GetHead
     * Vraci ukazatel na prvni polozku ve fronte, ktera je vzdy zaroven polozkou
     * s nejmensi hodnotou.
     * @return Vraci ukazatel na 1./nejmensi polozku fronty, nebo NULL, pokud je
     * fronta prazdna.
     */
    Element_t *GetHead();

    //
    //============================================================================//

protected:
    Element_t *head = nullptr;
};

#endif // TDD_CODE_H_
