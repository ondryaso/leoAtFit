# Základy počítačové grafiky
### Ak. rok 2019/2020. [Karta předmětu](https://www.fit.vut.cz/study/course/13375/.cs).

## Projekt
Hodnocení: 30/30

Projekt byl v tomto roce poněkud rozšířen, protože jsme kvůli koroně přišli o tři cvika. Cílem bylo naimplementovat základní softwarovou vykreslovací pipeline (na styl OpenGL) a poté pomocí ní vykreslit model králíka s Phongovým osvětlovacím modelem a procedurálně generovanou texturou.

Implementovaná GPU obsahuje zhruba:
- obslužné funkce pro správu bufferů, shader programů a dalších záležitostí
- sestavování vertexů
- clipping
- perspective division, viewport transformation
- rasterizace (Pinedův algoritmus)

Moje implementace není nikterak úžasná, efektivní, ale prý byla pořád rychlejší než Miletova referenční… a funguje. 