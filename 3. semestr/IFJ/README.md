# Formální jazyky a překladače
### Ak. rok 2020/2021. [Karta předmětu](https://www.fit.vut.cz/study/course/13981/.cs).

## Výpisky ke zkoušce
**Budou doplněny, až se zase dostanu ke scanneru.**

Obsah PDF:
- 1 (1a): Přehled pojmů (řetězec, jazyk)
- 2 (1b): Regulární výraz, konečný automat, bezkontextová gramatika
- 3 (2a): Zásobníkový automat
- 4 (2b): Syntaktická analýza zdola nahoru (Precedenční SA, tvorba precedenční tabulky)
- 5–6 (3): Syntaktická analýza shora dolů (LL tabulka; množiny First, Empty, Follow, Predict)
- 7 (4): Pumping lemma pro regulární jazyky, uzávěrové vlastnosti, hlavní rozhodnutelné problémy; Minimalizace KA, KA pro doplněk
- 8 (5a): Chomského normální forma, Greibachové normální forma; Pumping lemma pro bezkontextové jazyky, uzávěrové vlastnosti BKJ, rozhodnutelné a nerozhodnutelné problémy
- 9 (5b): Přehled jazyk-gramatika-model-tvar pravidel

Zkouška je vcelku jednoduchá, jen je zapotřebí nemít guláš v automatech a zafixovat si principy dělání různých věcí (zejména konstrukce tabulek pro syntaktické analýzy).

## Projekt
Hodnocení: full (15/15 + 5/5 bonusové body + 5/5 dokumentace + 5/5 obhajoba) + výhra v rychlostní soutěži

> Vytvořte program v jazyce C, který načte zdrojový kód zapsaný ve zdrojovém jazyce IFJ20 a přeloží jej do cílového jazyka IFJcode20 (mezikód). Jazyk IFJ20 je zjednodušenou podmnožinou jazyka Go, což je staticky typovaný imperativní jazyk.

_Celé zadání z roku 2020 pravděpodobně najdete ve WISu v souborech k předmětu._

Týmový projekt, všechno (kód, dokumentaci, prezentaci) najdete [v tomto repozitáři](https://github.com/Adda0/ifj20-go-compiler). \
Implementovaná rozšíření: BOOLTHEN, BASE, FUNEXP, MULTIVAL, UNARY. \
Kromě rozšíření ze zadání obsahuje naše implementace navíc základní zotavování z chyb a optimalizace vnitřního kódu.