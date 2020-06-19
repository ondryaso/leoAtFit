# Operační systémy
### Ak. rok 2019/2020. [Karta předmětu](https://www.fit.vut.cz/study/course/13332/.cs).

## Materiály ke studiu
IOS má vcelku dobré prezentace, doporučuji projet si to spolu se záznamy na rychlost alespoň 2× (je to pak vlastně velmi zajímavý předmět). Jistý _kůň_ z přednášek a prezentací vytvořil vcelku ucelený soupis, který jsem se poté jal upravovat a rozšiřovat. Najdete jej [v tomto repozitáři](https://github.com/ondryaso/FIT-IOS-notes), dá se to brát jako taková studijní opora, budu rád za případné pull requesty.

V mém PDF s poznámkami je celý předmět nahuštěný na 9 A4.
- 1: operační systémy obecně, koncepce UNIXu, přerušení
- 2–3: souborové systémy – disky, externí/interní fragmentace, plánování diskových operací, žurnálování, COW, UNIX _fs_, i-uzly a další
- 4–5: procesy, plánovač procesů, signály, synchronizace procesů (pozor, bez definice deadlocku)
- 6: implementace monitoru pomocí semaforů (kód)
- 7: konceptuální implementace semaforu pomocí spinlocku (kód)
- 8–9: paměť, stránkování, virtualizace paměti, tabulky <sub>_tabulek tabulek tabulek_</sub> stránek 

Ke zkoušce si dobře projeďte democvika (a survival guides z pinů na Discordu).

## Projekt 1
Hodnocení: 15/15
> Cílem úlohy je vytvořit skript, který prozkoumá adresář a vytvoří report o jeho obsahu. Předmětem rekurzivního zkoumání adresáře je počet souborů a adresářů a velikosti souborů. Výstupem skriptu je textový report.

Použití: `dirgraph [-i REGEX] [-n] [DIR]`. \
Pokud byl zadán adresář `DIR`, bude se prozkoumávat on, jinak se prozkoumává aktuální adresář. S přepínačem `-i` bude skript ignorovat soubory i adresáře, jejichž název odpovídá _rozšířenému regulárnímu výrazu_ `REGEX`; tento reg. výraz nesmí pokrývat název kořenového adresáře. Přepínač `-n` nastavuje normalizaci histogramu (= vleze se na šířku terminálu).

Tento projekt je zaměřen na práci se shellem. V zadání je uvedeno: `Referenční stroj neexistuje. Skript musí běžet na běžně dostupných OS GNU/Linux a *BSD.`, z čehož nepřímo vyplývá, že je nutné psát skript tak, aby byl „POSIX-compliant“, respektive využívat pouze prostředků běžného Bourne shellu (`/bin/sh`). Je to poněkud bolest, třeba proto, že `sh` neumí (rozumně) pracovat s poli. Pravděpodobně by to prošlo i s _bashismy_, ale asi s nějakou bodovou ztrátou. Moje řešení využívá rekurze, se kterou si po svém prochází adresáře (viz funkce `walk_dir`) – existují řešení využívající `find`, ale to s sebou nese různé problémy. Není to dvakrát rychlé, ale funguje to spolehlivě.

## Projekt 2
Hodnocení: 15/15
> Implementujte v jazyce C modifikovaný synchronizační problém Faneuil Hall Problem. Existují dva druhy procesů: přistěhovalci a jeden soudce. Přistěhovalci musí čekat ve frontě, vstoupit do soudní budovy, zaregistrovat se a pak čekat na rozhodnutí soudce. V určitém okamžiku vstoupí soudce do budovy. Když je soudce v budově, nikdo jiný nesmí vstoupit dovnitř ani odejít z budovy. Jakmile se všichni přistěhovalci, kteří vstoupili do budovy, zaregistrují, může soudce vydat rozhodnutí. Po vydání rozhodnutí si přistěhovalci vyzvednou certifikát o občanství. Soudce odejde z budovy v určitém okamžiku po vydání rozhodnutí. Poté, co přistěhovalci vyzvednou certifikát, mohou odejít. Přistěhovalci vstupují do budovy jednotlivě a také se jednotlivě registrují. Soudce vydává rozhodnutí pro všechny registrované přistěhovalce naráz. Certifikáty si imigranti vyzvedávají nezávisle na sobě a přítomnosti soudce v budově.

Použití: `./proj2 total_immigrants max_immigrant_spawn_time max_judge_enter_time max_certificate_pickup_time max_judge_confirmation_time`.\
Program pracuje s procesy, ne vlákny. Hlavní proces vytváří jeden proces pro soudce a proces, immigrant spawner, který vytváří další procesy, imigranty. Každý proces vykonává své akce a současně zapisuje informace o nich do výstupního souboru. Součástí každého záznamu o akci je pořadové číslo.

Tento projekt je zaměřen na synchronizaci procesů. Naprosto stěžejním čtením je [The Little Book of Semaphores](http://greenteapress.com/semaphores/LittleBookOfSemaphores.pdf). Nejen že tam je prakticky vyřešený celý tento problém (a spousta dalších), ale hlavně doporučuji opravdu si přečíst celé kapitoly 1 až 4, pomůže vám to k pochopení problému (problémů) synchronizace procesů obecně.