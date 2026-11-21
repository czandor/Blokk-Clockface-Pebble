# Ellenőrzési napló — Blokk (Pebble Time, Pebble Time Round, Pebble Time 2 & Pebble Round 2)

## 1.1.1 (2026-11-21)

### Fizikai Pebble Time Round-on mérve (`192.168.130.135`, ideiglenes `time_ms` + `APP_LOG` mérőbuilddel)
- **Előtte (1.1.0):** minden másodpercben teljes rajzolás, ~55 ms képkockánként (számjegyek 8–12 ms, árnyék/háttér 8–12 ms, dátum 12–16 ms, 4 mező 16–20 ms; az óra időzítője ~4 ms-os lépésekben mér).
- **Utána (1.1.1):** gyorsítótárból 0–4 ms a másodpercenkénti képkocka. Teljes rajzolás csak percváltáskor vagy változáskor, ~55 ms; ennek nagyobb részét a firmware szövegrajzolása adja.
- A végleges `Blokk-1.1.1.pbw` telepítve az órára.

### Emulátoron és automatizált tesztekkel
- **Build:** `pebble clean` + `tools/build.ps1` sikeres mind a négy platformra (`dist/Blokk-1.1.1.pbw`).
- **Egységtesztek:** sikeres; új ellenőrzés: friss telepítéskor `basalt`/`chalk`/`gabbro` esetén `FIELD2 = 7` (`ACT`), `FIELD6 = 9` (`SEC`), `FIELD3 = 10` (`VIB`); Emery-n továbbra is `FIELD2 = 2` (`BPM`).
- **Pixelmenet-egyezés:** a táblás gradiens és a soronkénti sarokszélesség Pythonban kimerítően összevetve az eredeti képlettel, mind a 4 kijelzőméreten és mind a 9 színen: 0 eltérő pixel (1,5 millió pixel).
- **Képernyőkép-összevetés az 1.1.0-val (14:25, türkiz):** Emery pixelre azonos. Gabbro és Basalt csak a középső mező területén tér el (`SEC` → `ACT`, illetve Basalt-on a töltésjelzés).
- **Chalk:** firmware-konzollal ellenőrizve, hardfault nincs. Verem-tartalék a teljes rajzolás legmélyebb pontján ~270 B (a mérőkód saját változóival együtt mérve).

## 1.1.0 (2026-11-21)

### Emulátoron és automatizált tesztekkel ellenőrizve
- **Build:** `pebble clean` + `tools/build.ps1` sikeres `basalt` + `chalk` + `emery` + `gabbro` célra (csak a szokásos `RWX` linker-figyelmeztetés). RAM-lábnyom a végleges buildben: Gabbro `8 203 B`, Emery `9 103 B`, Chalk `8 019 B`, Basalt `8 051 B`.
- **Egységtesztek (`node tools/test.cjs`):** sikeres; új ellenőrzés: `targetPlatforms = basalt, chalk, emery, gabbro`, a telefonos alapértelmezett `FIELD3 = 10` (`VIB`), a Clay-mock az `@rebble/clay` csomagnévre figyel.
- **Chalk (Time Round) összeomlás és javítás:** az első `chalk` build a Time Round emulátoron és Andor valódi óráján is újraindulási hurokba került. A firmware soros konzolja szerint hardfault: `PC = 0x08062754` (firmware), `MMFAR = 0x20018000` (`DACCVIOL`), `Task: App <Blokk>`. Felezéses tesztekkel kizárva: a fontméret (a `FONT_TIME_70` legnagyobb glifje 179 B < 256 B, és a 60-as fonttal is elszállt), a pozíció, a szóköz-glif, a közös font-erőforrás és a Health. `APP_LOG` és verem-feltöltéses (`0xA5`) mérés alapján verem-túlcsordulás: az update proc belépésekor csak ~1,6 KB szabad verem van, ennek ~1,4 KB-ját a firmware szövegrajzolója használja. Javítás után 9+ egymás utáni képkocka hiba nélkül, a legkisebb mért szabad verem ~300 B. Töltés közben (`emu-battery --charging`) is ellenőrizve, hardfault nélkül.
- **Chalk elrendezés (`docs/chalk-case-*.png`):** `14:25`, a leghosszabb dátum (`SEPTEMBER 30 WED`) `22:08`-kor és `12:11` a 180×180-as körön belül.
- **A rajzolókód átszervezése után újra ellenőrizve:** Gabbro, Emery és Basalt emulátoron hiba nélkül fut. Basalt-on töltés közben a `BTT 070+` kiírás is látszik.
- **Töltésjelzés (`+`):** a `BTT` érték mögött, ha az óra töltőn van (`is_charging || is_plugged`). Emulátoron Basalt-on és Chalk-on ellenőrizve; Andor kérésére nem készült róla új áruházi kép.
- **Gabbro (Round 2) emulátor (`docs/gabbro-case-*.png`, `docs/gabbro-vib.png`):**
  - Széles (`20:08`), keskeny (`11:11`) és vegyes (`14:25`, `08:47`) számjegyek a körön belül; a 45°-os árnyék és a háttér csak a látható körben rajzolódik.
  - 5 jegyű lépésszám (`12345`) és a 3 soros mezők a kör alsó részén is a kijelzőn belül maradnak.
  - 12 órás mód `AM` (`10:09`) és `PM` (`09:41`) felirattal: a felirat saját árnyékot vet, és nem ér bele a fő árnyékba (ideiglenes `companion.js` alapértékkel, utána visszaállítva).
  - Kék gradiens háttér (`18:36`) ditherelve a körön belül.
  - Rázásra megjelenő második mezőkészlet (`CAL`, `SLP`, `DST`) a `emu-tap` paranccsal.
- **`VIB` mező mindhárom emulátoron (`docs/basalt-vib.png`, `docs/emery-vib.png`, `docs/gabbro-vib.png`):** alapértelmezettként a jobb felső helyen, `VIB ON` kiírással.

### Fizikai órán
- **Pebble Time 2 (`192.168.130.135`):** a `VIB`-es `Blokk-1.1.0.pbw` sikeresen telepítve; Andor élőben ellenőrizte („jó minden”).
- **Pebble Time Round (ugyanazon a telefonon, `192.168.130.135`):** az első `chalk` build Andor óráján is összeomlott; a verem-javított és a töltésjelzéses végleges build telepítve, Andor szerint „működik, nem crashel már”. Andor szerint az óralap a Time Round-on kissé lassú, de használható. A teljes képernyős pixelmenet minden frissítéskor lefut, és alapból a `SEC` mező másodpercenként frissít; ez optimalizálható.
- **Még ellenőrzendő:** a `VIB OFF` állapot valódi Néma időben (az emulátorban a Néma idő parancssorból nem kapcsolható, így az `OFF` kiírás csak kódszinten ellenőrzött), valamint a teljes Round 2 elrendezés fizikai Pebble Round 2 órán (ilyen eszköz nem állt rendelkezésre).

## 1.0.1 (2026-11-21)

- **Build és tesztek:** `tools/build.ps1` (`emery` + `basalt` warning nélkül) és `node tools/test.cjs` sikeres (`dist/Blokk-1.0.1.pbw`).
- **Emery és Basalt emulátor képernyőképek:** lekerekített fekete sarkok (`R=16` / `R=12`) és élesebb alsó felirat/szám betűméretek (`17/24` Emery, `13/17` Basalt) ellenőrizve, áruházi képek és egyszínű türkiz banner újragenerálva.
- **Fizikai Pebble Time 2 (`192.168.130.135`):** `Blokk-1.0.1.pbw` sikeresen telepítve.

## 1.0.0 (2026-11-21)

### Emulátoron és automatizált tesztekkel ellenőrizve (Emery)
- **Build (`tools/build.ps1`):** Sikeres fordítás WSL `NineTimeSDK` alatt (`dist/Blokk-1.0.0.pbw`, `SHA256SUMS.txt`). Erőforrásméret: `15 130 B / 256 KB`, RAM lábnyom: `7 163 B / 128 KB`.
- **Egységtesztek (`node tools/test.cjs`):** Manifest (`emery`, egyedi UUID), Clay `config.json` kulcslefedettség és `companion.js` (`ready` / `webviewclosed` / `localStorage`) ellenőrizve és sikeres.
- **Emery emulátor képernyőképek (`docs/emery-first.png`, `docs/emery-check.png`, `docs/emery-24h.png`):**
  - Dátumsor (`NOVEMBER 21 SAT`), nagyméretű `HH:MM` idő és kétkörös kettőspont pontos középre igazítása.
  - 45°-os fekete hosszú árnyék megjelenítése a számok alatt és mögött.
  - 4 információs mező (`STP`, `ACT`, `BPM`, `BTT`) elhelyezése az árnyéksávon belül, átfedés nélkül.
  - Élő lépésszám (`pebble emu-steps 5879`) és pulzus (`pebble emu-heart-rate 64`) frissülése.

- **Basalt emulátor képernyőkép (`docs/basalt-24h.png`):**
  - 144×168-as elrendezés ellenőrizve (`NOVEMBER 21 SAT`, `14:34`, 45°-os árnyék, 3 soros `STP` / `ACT` / `SEC` / `BTT` mezők), `13 622 B` erőforrás, `7 299 B / 64 KB` RAM.

### Fizikai Pebble Time 2 órán még ellenőrzendő
- Valós csuklórázás érzékenysége, a 60 ms-os rezgés érzete (Néma idővel és anélkül), valamint a rázásra induló HR-mérés és a `BPM` felirat 500 ms-os villogása (`GColorDarkGray` ↔ `GColorWhite`).
- A 9 egyszínű és 9 ditherelt gradiens háttér olvashatósága valódi transzflektív kijelzőn, napsütésben és háttérvilágítással.
- Telefonos Pebble app Clay beállításoldalának megnyitása, mentése és a beállítások fennmaradása óralap-újraindítás után.
