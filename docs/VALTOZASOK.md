# Változásnapló — Blokk (Pebble Time, Pebble Time Round, Pebble Time 2 & Pebble Round 2)

## 1.1.1 (2026-11-21)

- **Gyorsabb rajzolás, főleg a Pebble Time Round-on.** Képkocka-gyorsítótár: a háttér, a számjegyek, a 45°-os árnyék, a dátum és a nem másodpercenként változó mezők egyszer rajzolódnak meg, és a kép a heapen eltárolódik. Amíg ezek közül semmi nem változik (perc, szín, beállítás, mezőértékek), a másodpercenkénti frissítés csak visszamásolja a képet, és a `SEC` mezőt rajzolja rá. Valódi Time Round-on mérve ~55 ms helyett 0–4 ms képkockánként.
- A teljes rajzolás pixelmenete előre számolt táblákkal fut (gradiens: oszloponkénti `tx`, csatornánkénti kvantálás és dither-küszöb; lekerekített sarkok: soronkénti szélesség), függvényhívás és osztás nélkül, bitre azonos eredménnyel.
- **Új alapértékek pulzusmérő nélküli órákon (Pebble Time, Time Round, Round 2):** a 2. mező `SEC` helyett `ACT` (aktív percek), a `SEC` pedig a rázásra megjelenő készlet 6. mezőjébe került (eddig `Nothing`). Így alapból percenként frissül az óralap. A már elmentett beállításokat ez nem írja felül.

## 1.1.0 (2026-11-21)

- Új platform: **Pebble Time Round (`chalk`, 180×180 kerek)** a Round 2 elrendezés ~0,69-szeres kicsinyítésével (`FONT_TIME_70`, `FONT_DATE_17`, `FONT_INFO_18`; a `FONT_WDAY_11`, `FONT_LABEL_13`, `FONT_AMPM_18` közös a Pebble Time-mal). Pulzusmérő itt sincs, ezért a `SEC` az alapértelmezett 2. mező.
- **Töltésjelzés:** a `BTT` érték mögött `+` jel jelenik meg, ha az óra töltőn van (pl. `085+`), minden platformon. A `+` karakter bekerült a `FONT_INFO_*` fontokba.
- **Javítva: verem-túlcsordulás a Time Round-on.** Az óralap első rajzolásakor hardfaulttal (MPU-sértés az app RAM elején, `0x20018000`) újraindította az órát, ami a beállított óralappal végtelen újraindulási hurokká vált. Ok: a Time Round firmware-ben (v4.3) az app verme ~2 KB, a firmware szövegrajzolója ennek ~1,4 KB-ját használja, a 180 bájtos helyi árnyékpuffer és az összevont (inline) rajzolófüggvények pedig a maradékot is elvitték. Javítás: az árnyéksor-puffer és a szövegpufferek statikus memóriába kerültek, a rajzolólépések külön (`noinline`) függvényekben futnak. Mért tartalék a legmélyebb ponton: ~300 bájt (előtte negatív). A többi platformot is érinti a kód, ott is működik.

- Új platform: **Pebble Round 2 (`gabbro`, 260×260 kerek)**. Saját `American Captain` méretkészlet (`FONT_TIME_100`, `FONT_DATE_24`, `FONT_WDAY_15`, `FONT_INFO_26`, `FONT_LABEL_18`, `FONT_AMPM_26`) és a kör alakú kijelzőhöz igazított elrendezés: dátum fent, nagy `HH:MM` középen, a 3 soros információs mezők a 45°-os árnyékban.
- Kerek kijelzőn nincs sarokmaszk (`CORNER_R = 0`): a kijelző széle adja a kártya körvonalát. A framebuffer-menet a soronkénti `min_x`/`max_x` tartományt használja, így az árnyék és a háttér pontosan a körön belül rajzolódik.
- A Round 2-ben nincs pulzusmérő: a HR-mérő kód fordítási időben ki van zárva (`HAS_HR_SENSOR = 0`), az alapértelmezett 2. mező `SEC`, a 6. mező `Nothing` (a C-alapértékek és a telefonos `companion.js` is a `basalt` mellett a `gabbro` platformot is pulzusmérő nélkülinek kezeli).
- Új **`VIB`** információs mező minden platformon: `ON`, ha az óra rezeghet, `OFF`, ha a Néma idő aktív. Ez lett az alapértelmezett 3. (jobb felső) mező az `ACT` helyett; az `ACT` továbbra is választható, a már elmentett beállítások nem változnak. A mező percenként és az óralap fókuszba kerülésekor frissül.
- A `pebble-clay` 1.0.4 nem tartalmaz `gabbro` binárist, ezért a karbantartott **`@rebble/clay` 1.1.0** csomagra váltottunk (ugyanaz a Clay API; `aplite`–`gabbro` és `flint` támogatással).

## 1.0.1 (2026-11-21)

- Lekerekített fekete kijelzősarkok az eredeti Fitbit Blokk keretéhez igazítva (`CORNER_R = 16` Emery-n, `CORNER_R = 12` Basalt-on), közvetlenül a framebuffer-menetben, extra RAM- és flash-igény nélkül.
- Élesebb, monokróm pixelrácsra optimalizált alsó mezőfelirat- és számméretek (`FONT_LABEL_17` + `FONT_INFO_24` Emery-n, `FONT_LABEL_13` + `FONT_INFO_17` Basalt-on), hogy a `BPM`, `STP`, `BTT`, `ACT` betűi tisztán, torzulás nélkül jelenjenek meg.
- Frissített áruházi képernyőképek és egyszínű türkiz hero-órás banner (`store/banner-720x320.png`).

## 1.0.0 (2026-11-21)

- Többplatformos natív C port a Fitbit **Blokk** óralapból **Pebble Time 2 (`emery`, 200×228)** és **Pebble Time / Time Steel (`basalt`, 144×168)** platformokra.
- Basalt (`Pebble Time`) adaptáció: 144×168-ra méretezett `American Captain` betűkészlet és 3 soros árnyékkövető elrendezés, pulzusszenzor hiányában alapértelmezett `SEC` mezővel (`BPM` helyett) és üres (`Nothing`) másodlagos 6. mezővel.
- Eredeti `American Captain` betűtípus beépítése 6 natív Pebble méretben (`FONT_TIME_84`, `FONT_DATE_22`, `FONT_WDAY_14`, `FONT_INFO_22`, `FONT_LABEL_15`, `FONT_AMPM_24`).
- Egymenetes, pontos 45°-os fekete hosszú árnyék a fő időszámok, a kettőspont és a 12 órás `AM`/`PM` jelző mögött.
- Mind a 9 eredeti színséma egyszínű (e-papír kontrasztra hangolt) és 4×4 Bayer ditherelt átlós gradiens módban, mind a 6 színválasztási móddal (napszak szerint, fix, napi/órai/perci véletlen, rázásra véletlen).
- 4 + 4 (rázásra 20 mp-re megjelenő) konfigurálható információs mező a magasabb 200×228-as kijelzőhöz és a 45°-os árnyékháromszöghöz igazított 3 soros elrendezésben (1. sor: bal + jobb mező; 2–3. sor: az alsó két mező egymás alatt): `STP`, `BPM`, `BTT` (mindig vagy ≤30%), `DST` (km/mi), `SLP`, `CAL`, `ACT`, `SEC`, `Nothing`.
- Igény szerinti pulzusmérés csuklórázásra (`Measure heart rate on shake`, max. 45 mp, 1 Hz, 2 konzisztens friss minta, 60 mp eredmény-megtartás), mérés közben 500 ms-onként halvány (`GColorDarkGray`) ↔ normál (`GColorWhite`) villogó `BPM` felirattal.
- Offline Clay telefonos beállításoldal, automatikus `MINUTE_UNIT`/`SECOND_UNIT` váltás, Néma időt tiszteletben tartó rázásrezgés.
- Appstore-csomag (`store/`): 4 Emery (`200×228`) + 2 Basalt (`144×168`) emulátoros képernyőkép, `banner-720x320.png` és kész áruházi leírás (`STORE_TEXT.md`).
