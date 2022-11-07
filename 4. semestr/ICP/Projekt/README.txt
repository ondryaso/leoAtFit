ICP projekt - varianta 1 - MQTT Explorer/Dashboard/Simulator
===============================================================================

Autoři: František Nečas (xnecas27)
        Ondřej Ondryáš (xondry02)

Tento projekt implementuje grafickou aplikaci dle zadání skládající se ze tří
celků:
 - MQTT Exploreru pro sledování provozu na MQTT brokeru s možností vytvářet
   nová temáta, zasílat k nim zprávy a sledovat celou historii u tématu.
   (plně funkční se všemi vlastnostmi dle zadání).
 - MQTT Dashboardu pro zobrazování nejnovější zprávy přijaté k tématu ve formě
   pojmenovaných widgetů.
 - MQTT Simulátoru pro testování a demonstraci funkčnosti, který simuluje
   reálný síťový provoz, například v chytré domácnosti, a je tedy možné
   vytvářený provoz sledovat například MQTT Explorerem.

-------------------------------------------------------------------------------
Poznámky:
 - Implementace modelu pro stromovou strukturu byla inspirována oficiální
   dokumentací Qt:
   https://doc.qt.io/qt-5/qtwidgets-itemviews-editabletreemodel-example.html

 - Implementace MQTT callbacku byla inspirována příklady v Github repozitáři
   knihovny Eclipse Paho MQTT C++, převážně:
   https://github.com/eclipse/paho.mqtt.cpp/blob/master/src/samples/async_subscribe.cpp

 - Ikonky ve složce assets byly převzaty ze stránky https://www.iconfinder.com/
   ze sekce pro free ikony, jejich použití se shoduje s licenčními podmínkami
   této stránky (osobní a vzdělávací účely) a tedy bylo možné je bezplatně použít.

 - Rozpoznávání přijímaných zpráv je mírně omezeno/zjednodušeno:
     - Obrázky jsou podporovány ve formátu JPG a PNG.
     - Zpráva je brána jako binární data, pokud obsahuje nulový byte (toto není
       vůbec perfektní, ale detekce binárních dat záleží na daném formátu a
       Explorer zcela postrádá kontext nutný k rozeznání).
     - JSON je rozpoznán dle uvozujícího znaku řetězce pro zjednodušení, neboť
       standardní knihovna nemá knihovnu pro manipulaci JSON a cílem projektu
       není implementace plnohodnotného JSON parseru (navíc JSON nemá
       v aplikaci žádnou roli kromě koncovky souboru payload při ukládání).

 - Dashboard nepodporuje vkládání libovolného textu k libovolným tématům; zasílání
   zpráv zpět na server demonstruje pouze widget "přepínač".

 - Dashboard využívá rozvržení "FlowLayout", které bylo převzato z příkladu v dokumentaci Qt:
   https://doc.qt.io/qt-5/qtwidgets-layouts-flowlayout-example.html

 - Je možné mít paralelně spuštěno několik oken Dashboard, každé s vlastní konfigurací.

 - Simulátor zasílá zprávy podle načtené textové konfigurace. Její příklad je uveden v souboru
   examples/simulator_config. Simulovaná komponenta je definována třemi řádky v souboru: na prvním
   je uveden typ, na druhém téma, na třetím frekvence zasílání zprávy. Povolené typy jsou:
     temperature, watts, relay, camera, tempcontrol
   Textové zprávy jsou generovány náhodně, v případě komponenty "camera" je nutné zvolit
   soubor s obrázkem, který se má zasílat.