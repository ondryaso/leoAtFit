# Základy programování
### Ak. rok 2019/2020. [Karta předmětu](https://www.fit.vut.cz/study/course/13376/.cs).

## Projekt 1
Hodnocení: 5/5 (+prémie)
> Snad všichni známe telefonní seznam v chytrých telefonech. Vyhledávání tam většinou funguje i za pomocí numerické klávesnice. Uživatel zadává posloupnosti číslic, ale každým stiskem klávesy myslí jeden ze znaků, které daná čislice reprezentuje (například číslo 5 reprezentuje samotnou 5 či jedno z písmen j, k nebo l). Telefon pak filtruje telefonní seznam podle dané posloupnosti, přičemž se snaží v seznamu najít libovolný kontakt, jehož jméno nebo telefonní číslo obsahuje sekvenci (nepřerušenou nebo rozdělenou) zadaných znaků.

## Projekt 2
Hodnocení: 7/7
> Mějme jednoduché sériové zapojení diody a rezistoru. Známe Shockleyovu rovnici a jeho konstanty. Napište funkci, která pro dané vstupní napětí U_0 a odpor rezistoru R najde pracovní napětí diody odpovídající zadané přesnosti. Dále napište program, který na základě těchto vstupních parametrů na výstup vytiskne pracovní bod diody, tedy napětí a proud. (...) Implementujte algoritmické schema pro výpočet posloupnosti využívající metodu půlení intervalu. Ukončující podmínka bude odpovídat absolutní požadované přesnosti výsledku (epsilon). Napište funkci diode, která pomocí schematu z 2. podúkolu a vzorce z 1. podúkolu hledá hodnotu napětí U_p. Počáteční interval napětí bude od 0 do U_0.

## Projekt 3
Hodnocení: 9,35/10 (chyby opraveny)
> Vytvořte program, který v daném bludišti a jeho vstupu najde průchod ven. Bludiště je uloženo v textovém souboru ve formě obdélníkové matice celých čísel. Cílem programu je výpis souřadnic políček bludiště, přes které vede cesta z vchodu bludiště do jeho východu.

Mé řešení projektu není nikterak extrémně promyšlené či optimální, ale splňuje všechny požadavky. Velkou část kódu samozřejmě tvoří různé kontroly. Bonusové zadání se mi nechtělo dělat.

V hodnocení mi bylo odebráno 0,5 bodu, protože jsem nekontroloval bludiště na záporné velikosti (na řádku 182 jsem měl v podmínce pouze `==`, ne `<=`). 0,15 bodu bylo odebráno za výpis některých chyb na stdout (na ř. 546, 560 a 570 jsem měl pouze `printf`).