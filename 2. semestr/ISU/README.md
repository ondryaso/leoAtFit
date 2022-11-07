# Programování na strojové úrovni
### Ak. rok 2019/2020. [Karta předmětu](https://www.fit.vut.cz/study/course/210277/.cs).

**Všechny moje (programové) materiály k tomuto předmětu najdete v repozitáři [isu-examples](https://github.com/ondryaso/isu-examples).**

## Stream
K tomuto předmětu jsem dělal stream **před třetím testem**, který se zabývá hlavně prací s FPU a voláním funkcí (a tedy především správným používáním zásobníku). Najdete jej na [youtube.com/watch?v=WOegtgYE7X8](https://www.youtube.com/watch?v=WOegtgYE7X8).

## Poznámky ke zkoušce
Obsah PDF:
- 1–4: přehled instrukcí
- 5–6: teorie – registry, formát instrukce
- 7: výjimky a interrupty
- 8: flagy (EFLAGS, FPU)
- 9: převody soustav, kódování čísel v binárce
- 10: formát čísla s plovoucí desetinnou čárkou v FPU

## Sčítání pro začátečníky
Prezentace vysvětlující, jak funguje sčítání v binární soustavě, jak funguje doplňkový kód a jak se dá poznat přetečení při bezznaménkovém (carry) a znaménkovém (overflow) sčítání.

## Zkouška
Zkouška se už roky prakticky nezměnila – na Fitušce najdete zkoušky z roku 2015/16 a 2016/17, pokud zvládnete napsat tyhle, zvládnete dost pravděpodobně i tu svou. Co je třeba znát:
- **Aritmetické instrukce**: ADD, SUB, **ADC**, **SBB**, **(I)MUL** a **(I)DIV** 
- **Rotace a posuvy**: **SAR**, SAL, **SHR**, SHL, ROR, ROL, **RCL**, **RCR**
- **Instrukce FPU**: pozor na instrukce s **R** na konci (FDIVR, FSUBR), které fungují _naopak_, a na instrukce s **P**, které popují zásobník (odstraní `st0`)
- **Řetězové instrukce**: určitě tam budou – MOVSx, CMPSx, LODSx, STOSx, možná SCASx, nezapomeňte na nastavování direction flagu – **STD**, **CLD**; a je třeba chápat jak funguje prefix **REP((N)E)**.
- **Skoky**: JMPcc, **LOOP((N)E)**
- **Volání funkcí a práce se zásobníkem**: konvence cdecl, stdcall, pascal, fastcall!
- **Reprezentace čísel**: naučte se pořádně, rychle, z hlavy převádět dekadická a binární čísla (v doplňkovém kódu), odčítat a sčítat binární čísla a taky **reprezentaci čísla v plovoucí řádové čárce** (zejména 32b float)
- **Flagy**: CF, OF, SF, ZF, PF; pak nějaké flagy v FPU (co je v control registru, tag registru, stavovém registru…)
- **Další teorie**: aspoň základní ponětí, z čeho se skládají instrukce, jak se tvoří adresy, jaké registry jsou k čemu…