# Ellenőrzési napló — Blokk (Pebble Time, Pebble Time 2 & Pebble Round 2)

## 1.1.0 (2026-11-21)

### Emulátoron és automatizált tesztekkel ellenőrizve
- **Build:** `pebble clean` + `tools/build.ps1` sikeres `basalt` + `emery` + `gabbro` célra (csak a szokásos `RWX` linker-figyelmeztetés). Erőforrás / RAM: Gabbro `16 585 B` / `7 523 B`, Emery `15 261 B` / `8 475 B`, Basalt `13 681 B` / `7 475 B`.
- **Egységtesztek (`node tools/test.cjs`):** sikeres; új ellenőrzés: `targetPlatforms = basalt, emery, gabbro`, a telefonos alapértelmezett `FIELD3 = 10` (`VIB`), a Clay-mock az `@rebble/clay` csomagnévre figyel.
- **Gabbro (Round 2) emulátor (`docs/gabbro-case-*.png`, `docs/gabbro-vib.png`):**
  - Széles (`20:08`), keskeny (`11:11`) és vegyes (`14:25`, `08:47`) számjegyek a körön belül; a 45°-os árnyék és a háttér csak a látható körben rajzolódik.
  - 5 jegyű lépésszám (`12345`) és a 3 soros mezők a kör alsó részén is a kijelzőn belül maradnak.
  - 12 órás mód `AM` (`10:09`) és `PM` (`09:41`) felirattal: a felirat saját árnyékot vet, és nem ér bele a fő árnyékba (ideiglenes `companion.js` alapértékkel, utána visszaállítva).
  - Kék gradiens háttér (`18:36`) ditherelve a körön belül.
  - Rázásra megjelenő második mezőkészlet (`CAL`, `SLP`, `DST`) a `emu-tap` paranccsal.
- **`VIB` mező mindhárom emulátoron (`docs/basalt-vib.png`, `docs/emery-vib.png`, `docs/gabbro-vib.png`):** alapértelmezettként a jobb felső helyen, `VIB ON` kiírással.

### Fizikai órán
- **Pebble Time 2 (`192.168.130.135`):** `Blokk-1.1.0.pbw` sikeresen telepítve; a megjelenést Andor nézi élőben.
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
