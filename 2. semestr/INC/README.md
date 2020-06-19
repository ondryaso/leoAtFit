# Návrh číslicových systémů
### Ak. rok 2019/2020. [Karta předmětu](https://www.fit.vut.cz/study/course/13327/.cs).

Zvláštní předmět. Většinou zcela ignorovaný, na zkoušku se dá naučit ze streamů, ale aspoň mi _na rychlost 2×_ přišel vcelku zajímavý.

## Výpisky z přednášek
Obsah PDF:
- 1: základní logické členy/operace v Booleově algebře, NDF, NKF
- 2: Shefferova algebra (NAND), Piercova algebra (NOR), rozklady log. funkcí, prahová log. funkce
- 3–4: minimalizace pomocí Quine-McCluskey a Petrickovy funkce
- 5–8: kombinační obvody (de-/kodér, de-/multiplexor, komparátor, sčítačky)
- 9–10: sekvenční obvody (RS, JK, T, D)
- 11: asynchronní čítač; VHDL
- 12: CMOS; SRAM, DRAM, funkční generátory v FPGA

## Projekt
> Cílem tohoto projektu je realizovat přístupový terminál na FITkitu. Úlohou FPGA čipu bude sledovat vstupy klávesnice, vyhodnocovat zadaný vstupní kód a vypisovat příslušné odezvy na LCD displeji. Všechno kromě konečného automatu FSM je již naimplementováno v jazyce VHDL. Cílem toho projektu je proto správně navrhnout a implementovat právě tento automat řídící zbývající části obvodu.

**Sežeňte si co nejdřív funkční FITkit**. Je to vskutku radostné zařízení. Projekt je vcelku jednoduchý, jen je třeba zamyslet se, jak funguje to VHDL, ve finále to ale všechno dává smysl. Načmárejte si ten stavový diagram na papír. Pak už je to v podstatě copy-paste. Pro _inspiraci_ je dobrá [tahle záležitost](https://github.com/thejoeejoee/VUT-FIT-INC-VHDL-FSM-generator). Diagram jsem dělal v [drawio](https://app.diagrams.net/), není to vůbec ideální, ale dá se to přežít.