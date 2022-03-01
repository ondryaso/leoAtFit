# Počítačové komunikace a sítě
### Ak. rok 2020/2021. [Karta předmětu](https://www.fit.vut.cz/study/course/224938/.cs).

## Projekt 1
Hodnocení: 19/20
> Cílem projektu je implementovat klienta pro triviální (read-only) distribuovaný souborový systém. Tento systém používá zjednodušenou verzi URL pro identifikaci souborů a jejich umístění. Systém pro přístup k souborům používá File Service Protocol (FSP). V projektu bude stačit implementovat pouze jeden typ požadavku, kterým je příkaz GET. Systém používá symbolických jmen, které jsou překládány na IP adresy pomocí protokolu Name Service Protocol (NSP). Tento protokol umožňuje získat IP adresu a číslo portu, kde daný souborový server běží.

Byla tam nějaká triviální chybka, kvůli které neprošel jeden test.

## Projekt 2 (varianta EPSILON)
Hodnocení: 19/20
> Navrhněte a implementujte CLI aplikace serveru a klienta v C/C++/C# pro protokol Simple FTP dle RFC 913 (13 b). Vytvořte relevantní manuál/dokumentaci k projektu (7 b). Výstupem programu budou dva programy ipk-simpleftp-server a ipk-simpleftp-client. Serverová aplikace bude na výstup vypisovat dotazy a odpovědi protokolu při komunikaci s klientem. Klientská aplikace vypisuje to samé, avšak při jejím ovládání očekává i prompt pro zadávání uživatelského vstupu kvůli interakci se serverem.

Bod ztracen za _slabší testování_.
