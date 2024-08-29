# ESP32_Golemio_ZivyObraz

Program pro ESP32 (v mém případě ESP32-C3 LPKit), které se přes Golemio API dotáže na aktuální odjezdy MHD v Praze.</br>
Ten samý program má ještě další část kódu, která se opět dotazuje na Golemio API, jaké sběrné kontejnery jsou v okolí a kdy se vyváží. Definuje se skrze #define v setup.h.</br>
</br>
Původním autorem kódu pro ESP32 je https://github.com/kkodl/MHD_GW_ESP32</br>
PDF https://github.com/kkodl/MHD_GW_ESP32/blob/main/docs/ODJEZDY%20MHD%20V%20PRAZE.pdf</br>
Ten napsal první verzi a dále kód vylepšuje. </br>
Já si tuhle verzi upravil dle svého pro mé vlastní potřeby.</br>
</br>
Účel: stáhnout data o MHD a poslat na Živý Obraz - https://zivyobraz.eu/, kde si je zobrazím na ePaper - https://www.laskakit.cz/laskakit-live-7-5-e-paper-stavebnice-s-wifi-pro---zivy-obraz-/?variantId=13529.</br>
To samé se svozem tříděnného odpadu - typ kontejneru v okolí a kdy se vyváží odpad. </br>
</br>
## MHD
Tzn. mám jen jeden spoj MHD, v kódu je ale možné vybrat až tři zastávky a jednoduchou úpravou kódu dále rozšířit.</br>
Spoje, které jsou klimatizované, jsou označeny "*" před časem odjezdu.</br>
Na Živý Obraz posílám i konečnou zastávku, protože ta se u nás mění.</br>
Odesílám: linku, konečnou stanici, odhadovaný čas odjezdu ze zastávky. Kód se dá rozšířit o další parametry jako bezbariérovost atp.</br>
</br>
## Svoz odpadu
Na základě GPS souřadnic a vzdálenosti (range) jsou vypsány dostupné kontejnery, zda jsou volně přístupné nebo patří k nějakému bytovému domu, o jaký typ se jedná (sklo, kov, papír atp) a kdy se sváží.</br>
Některé kontejnery obsahují čidlo zaplnění. Jednoduchou úpravou kódu můžete dostat informace i o zaplnění (nejčastěji kovy a sklo) a poslat také na Živý Obraz. </br>
</br>
## setup.h
Na co se má FW ptát - MHD a/nebo i svoz tříděnného odpadu?</br>
Vyplnit Golemio API (zaregistrovat se)</br>
Vyplnit vaši Wi-Fi</br>
Server Živý Obraz a jeho klíč</br>
HTTP_GET_INTERVAL_SECS - jak často se ESP32 probudí (v sekundách)</br>

## ESP32_Golemio_ZivyObraz.ino
Pozor na velikost JSONu! Pokud přijímáte hodně dat z Golemio API, je nutné zvětšit JsonBuffer (výchozí 6000 znaků, char json[6000];).</br>
Pro MHD vyplnit Stop ID (Stop_Ids) a počet odjezdů (Number_Of_Trips). Ve výchozím nastavení se používá jen jedna zastávka, zvolit jich můžete více -> stačí odkomentovat.</br>
</br>
Pro více informací - jak se zaregistrovat na Golemio API (https://api.golemio.cz/docs/openapi/), Živý Obraz (https://zivyobraz.eu/, https://wiki.zivyobraz.eu/) najdete u Karla</br>
https://github.com/kkodl/MHD_GW_ESP32</br>
Ten má verzi jak pro ESP32, tak i pro NodeRed. </br>
