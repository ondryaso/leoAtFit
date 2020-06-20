# Jazyk C
### Ak. rok 2019/2020. [Karta předmětu](https://www.fit.vut.cz/study/course/13315/.cs).

<img alt=":PePePepe:" width="24px" src="https://cdn.discordapp.com/emojis/490992251938406402.png?v=1" /> good guy. Ke zkoušce si dobře projděte piny na Discordu (bájný soubor _tryharding.zip_) a pozor na baity.

Kódy projektů jsou celkem dobře okomentovány.

## Projekt 1
Hodnocení: 15/15 | [Zadání](http://www.fit.vutbr.cz/study/courses/IJC/public/DU1.html.cs) (detaily se mění, ale veskrze jde vždycky o to samé)

První projekt se točí kolem práce s bitíky. Pokuste se to pochopit, bude to na zkoušce. Projekt musí být přeložitelný s `-O0`, což je problém, pokud v headeru vytvoříte `inline` funkci – vypnuté optimalizace neumí inlinovat. Řešení jsou dvě: buď z toho udělat `static inline` (což při kompilaci v podstatě zkopíruje tu funkci do každého modulu), nebo do jednoho z modulů přidat `extern` deklaraci, díky které to při kompilaci strčí ten kód do daného modulu. Já použil první řešení.

## Projekt 2
Hodnocení: 15/15 | [Zadání](http://www.fit.vutbr.cz/study/courses/IJC/public/DU2.html.cs) (nápodobně)

Ve druhém projektu implementujete [hashtable](https://cs.wikipedia.org/wiki/Ha%C5%A1ovac%C3%AD_tabulka), vyhledávací datovou strukturu, která vypadá jako velké pole, do kterého ukládám položky podle jejich hashe – výsledku nějaké matematické funkce, která pro libovolně dlouhý vstup vyplivne nějaké číslo – tohle číslo se pak použije jako index do pole. Jedno číslo může být vyplivnuto pro různé vstupy, proto je třeba navíc do pole neukládat položku přímo, ale mít v ní odkaz na nějaký zřetězený seznam. Užitečná věc.
