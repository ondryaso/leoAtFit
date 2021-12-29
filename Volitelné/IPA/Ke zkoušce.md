MMX:
- Co přináší, s čím pracuje, datové typy
- Jak se dá detekovat (princip CPUID)
- Maskovací aritmetika, (bez)znaménková saturační aritmetika
- Přesuny (MOVD, MOVQ)
- Sčítání, odčítání, násobení (PMULHW), PMADDWD
- Porovnávání
- _NE: konverze, expanze_
- Logické
- Posuny

SSE (nezkouší AVXové verze):
- Co přináší, s čím pracuje
- Obecně co je v řídicích registrech (že se tam dají nastavovat výjimky, zaokrouhlování apod. – ne konkrétní bity)
- Princip CPUID
- Přesuny: MOV[A|U][PS|PD], MOV[SS|SD]
- SHUFP[S|D] (pouze SSE)
- NE: extrakce (UNPCK)
- Aritmetika (všechno)
- Porovnávání (všechno, včetně konstant?)
- ADDSUB, HADD, HSUB
- _NE: konverze (CVT)_
- _NE: celočíselné („MMXové“) instrukce v SSE (tj. slidy 33–36)!_
- _NE: SSSE3 (slidy 39–42)_
- Dotproduct (DPP – slide 44)
- BLENDPS, BLENDVP[S|D]
- _NE: MMXové blendy – W, VB_
- _NE: všechno dál od slidu 49_

ARM:
- Přehledově obecné věci
- Podmínkové příznaky
- Adresování, jak se dělá zásobník…
- Všechny instrukce
- _NE přesně: formát instrukce_
- _NE: koprocesory_

NEON:
- LD instrukce
- Základní aritmetické instrukce (ADD, SUB, MUL, AND, ORR, ORN, EOR, NOT, ABS)

Chráněný režim:
- Zvolit selský rozum a naučit se, co uznáme za vhodné
- _Nelpí na detailech, spíš principy_
- Budou tam kreslicí otázky – různá schémata
- Jak se přepnout do chráněného režimu…
