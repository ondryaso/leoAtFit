# Návrh počítačových systémů
### Ak. rok 2020/2021. (https://www.fit.vut.cz/study/course/14003/.cs)

## Výpisky z přednášek
**Budou doplněny, až se zase dostanu ke scanneru.**

Obsah PDF:
- 1 (3): Kódy pro opravu a detekci chyb (paritní kód, ztrojení, Hammingův kód)
- 2 (4): ISA; Pipelining, RISC
- 3–4 (5): ALU; **Sčítačky**; Posuvy a rotace; **Násobičky**, **Boothovo překódování**
- 5–6 (6): Dělení, **SRT dělení**; Operace ve FP, **Newtonův iterační algoritmus**, CORDIC
- 7 (7): Paměti, DRAM
- 8: Flash paměti
- 9 (8): Řadiče, přerušení; Měřenní výkonnosti (Amdahlův zákon)
- 10: Spolehlivost

## Projekt 1 – Vigenèrova šifra
Hodnocení: 15/15
> V jazyce VHDL popište, programem Xilinx Isim odsimulujte a do binárního řetězce pro FPGA syntetizujte obvod realizující lehce modifikovaný algoritmus Vigenèrovy šifry.
> Vigenèrova šifra patří do kategorie substitučních šifer a její princip pro potřeby tohoto projektu bude spočívat v nahrazování každého znaku zprávy znakem, který je v abecedě posunut o hodnotu danou příslušným znakem šifrovacího klíče.
> Šifrování bude probíhat tak, že první znak klíče posouvá znak zprávy vpřed, druhý znak klíče posouvá znak zprávy vzad, číslice jsou nahrazovány znakem #. Pokud vychází posuv před písmeno A nebo za písmeno Z, uvažuje se cyklicky z opačného konce abecedy.

## Projekt 2 – Brainfuck interpreter
Hodnocení: 23/23
> Cílem projektu je implementovat pomocí VHDL procesor, který bude schopen vykonávat program napsaný v jazyce BrainFuck.