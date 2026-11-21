# Fejlesztői átadás — Blokk (Pebble Time, Pebble Time 2 & Pebble Round 2)

## Mi hol van

| Fájl | Feladat |
| --- | --- |
| `pebble/src/c/main.c` | Basalt (144×168), Emery (200×228) és Gabbro (260×260 kerek) natív C rajzolás, egymenetes 45°-os hosszú árnyék, 4×4 Bayer ditherelt gradiens, Pebble Health, rázás és beállításkezelés. |
| `pebble/src/pkjs/companion.js` | Telefonos offline Clay beállítások, `localStorage` mentés és AppMessage küldés. |
| `tools/prepare-project.cjs` | A `pebble/package.json` és `pebble/src/pkjs/config.json` forrása. Az AppMessage kulcsok sorrendje adja az azonosítókat, ezért új kulcsot mindig a végére kell fűzni. |
| `tools/build.ps1`, `tools/build.sh` | Windowsról futtatja a WSL `NineTimeSDK` buildet, majd a verziózott PBW-t elmenti a `dist/` alá `SHA256SUMS.txt`-vel. |
| `tools/Emulator-vezerlo.cmd`, `tools/emulator-panel.ps1`, `tools/emulator-pulse.sh` | Windows emulátor vezérlőpanel Emery-hez. |
| `tools/test.cjs` | Manifest, Clay konfiguráció és `companion.js` egységtesztek. |

Az elrendezés referenciája az `original/store/screenshots/versa/` képsorozat és az `original/source/` Fitbit-forrás. A `C:\andor\Fitbit.7z` archívum csak olvasható referencia.

## Fordítás és próba

A projekt gyökerében Windows alatt:

```powershell
powershell -ExecutionPolicy Bypass -File tools/build.ps1
node tools/test.cjs
```

**Fontos:** a waf nem érzékeli, ha egy font `characterRegex`-e változik a `prepare-project.cjs`-ben (a régi `.reso` marad). Ilyenkor előbb `pebble clean` kell a `pebble/` mappában, és csak utána a `build.ps1`.

Emulátoros telepítés (`NineTimeSDK` WSL-ben; `--emulator basalt`, `emery` vagy `gabbro`):

```bash
export PATH=/opt/ninetime-cli/bin:/usr/local/bin:/usr/bin:/bin
cd /mnt/c/andor/Pebble/Blokk/pebble
pebble install --emulator gabbro ../dist/Blokk-1.1.0.pbw
pebble emu-set-time --emulator gabbro 14:25:00   # fix időpont képernyőképhez
pebble emu-steps --emulator gabbro 5879
pebble screenshot --no-open --emulator gabbro ../docs/gabbro.png
```

Fizikai órára (telefonon bekapcsolt Developer Connection mellett):

```bash
pebble install --phone TELEFON_IP ../dist/Blokk-1.1.0.pbw
```

A telefonos JS induláskor elküldi az alapbeállításokat, ezért nem alapértelmezett opciók (pl. `AM`/`PM`, gradiens) emulátoros kipróbálásához ideiglenesen a `companion.js` `defaults` objektumát kell átírni, majd a próba után `git checkout`-tal visszaállítani.

## Elrendezési és működési döntések (`1.0.0`)

- **Natív 45°-os hosszú árnyék:** A `draw_time_and_shadow()` először fekete háttérre fehérrel rajzolja az óra számjegyeit (`FONT_TIME_84`), a kettőspont két körét és (12 órás módban, ha be van kapcsolva) a bal alsó `AM`/`PM` feliratot. Ezután egyetlen `200` bájtos stack-bufferrel (`shadow[SCREEN_W]`) fentről lefelé, jobbról balra végigmegy a framebufferen: minden fehér pixel és minden `(x-1, y-1)` árnyékos pixel alatt fekete árnyékot hagy, minden más pixelt pedig az aktív egyszínű vagy 4×4 Bayer ditherelt gradiens háttérszínnel tölt ki. Ez nulla extra flash- és RAM-igénnyel ad pengeéles, pontosan 45°-os árnyékot minden időpontra.
- **Fitbit → Pebble adaptációk:**
  - A Pebble Time 2 memóriakijelzője alapból Always-On, ezért a Fitbit-féle AOD/alvóidő/kijelző-halványítás beállítások nem szükségesek.
  - A Fitbit érintőkijelzős alsó mezőváltását Pebble-ön a **csuklórázás** (`accel_tap_service`) váltja ki: 20 másodpercre (vagy újabb rázásig) átkapcsol a másodlagos 4 információs mezőre, opcionális 60 ms-os rezgéssel (Néma időben alapból letiltva).
  - A Fitbit „kijelző bekapcsolásakor új szín” módja Pebble-ön a **rázásra új véletlen szín** (`BG_OPTION_SHAKE`) mód lett.
  - A Pebble Time 2-ben nincs barométer/emeletszámláló (`ELE`), helyette az **alvásidő** (`SLP`, órában, pl. `7.5`) választható a Pebble Health-ből.
- **Pebble Time (`basalt`, 144×168) és Pebble Time 2 (`emery`, 200×228) többplatformos támogatás:** Egy közös PBW mindkét platformot tartalmazza külön erőforrás-készlettel (`FONT_TIME_60` / `FONT_DATE_16` / `FONT_INFO_16` / `FONT_LABEL_12` / `FONT_WDAY_11` / `FONT_AMPM_18` Basalt-on). Mivel a Pebble Time-ban nincs pulzusmérő szenzor, Basalt-on az alapértelmezett 2. mező a másodperc (`SEC`), a másodlagos 6. mező pedig üres (`Nothing`), és a HR-mérő API-k fordítási időben (`HAS_HR_SENSOR = 0`) ki vannak zárva.
- **3 soros alsó elrendezés:** Mindkét platformon az alsó 4 mező a 45°-os árnyék dőlését követve 3 sorba került (1. sor: bal + jobb mező; 2–3. sor: az alsó két mező lépcsőzve az árnyékszél mellett).
- **Igény szerinti pulzusmérés rázásra (`HR_SHAKE`):** A `ProgrammersWatch` bevált PebbleOS 4.44 logikája került átvételre. Bekapcsolt **Measure heart rate on shake** opcióval csuklórázásra az óralap `1 Hz`-es mintavételt kér legfeljebb 45 másodpercre, és két egymást követő, az indításkoriból eltérő, legfeljebb ±5 BPM szórású friss nyers mintára vár. Mérés közben a `BPM` felirat 500 ms-onként villog halvány (`GColorDarkGray`) és normál fehér (`GColorWhite`) között. Az eredmény 60 másodpercig marad kijelezve, leálláskor / időtúllépéskor / bezáráskor a mintavétel visszatér a rendszer alapértelmezésére (`sample_period = 0`). Az állapot külön tartós kulcs (`HR_SHAKE_KEY = 3`) alatt tárolódik.
- **Lekerekített sarkok (`1.0.1`):** A framebuffer-menetben a négy sarok (`CORNER_R = 16` Emery-n, `12` Basalt-on) pontos egészszámos körívvel feketére maszkolódik (`is_outside_rounded_corner()`), visszaadva az eredeti Fitbit `border.png` kártyás megjelenését nulla plusz memóriaigénnyel.
- **Energiatakarékosság:** A tick timer csak akkor vált `SECOND_UNIT`-ra, ha a jelenleg látható 4 mező valamelyike `SEC`; egyébként `MINUTE_UNIT`-on fut.

## Pebble Round 2 és `VIB` mező (`1.1.0`)

- **Gabbro (Pebble Round 2, 260×260 kerek) elrendezés:** külön `#elif defined(PBL_PLATFORM_GABBRO)` ág a `main.c` elején. Betűméretek: `FONT_TIME_100`, `FONT_DATE_24`, `FONT_WDAY_15`, `FONT_INFO_26`, `FONT_LABEL_18`, `FONT_AMPM_26` (a `gabbro` font-glif korlátja 512 bájt, mint az Emery-é). A dátum (`DATE_Y = 26`) a kör felső, még elég széles sávjában van; az idő (`TIME_Y = 46`) a kör legszélesebb részén; a 3 soros mezők (`INFO_ROW_Y = {148, 175, 148, 202}`, `INFO_SPLIT_X = {106, 134, 196, 160}`) a 45°-os árnyékban, úgy, hogy a legalsó sor jobb széle (`y ≈ 232`) még a körön belül maradjon. Az `AM`/`PM` (`GRect(40, 178, 46, 34)`) a 2. információs sor magasságában, a bal oldali színes mezőben van, és a legszélesebb 12 órás kezdőszámjeggyel (`0`) sem ér bele a fő árnyékba.
- **Nincs sarokmaszk kerek kijelzőn:** `CORNER_R = 0`, az `is_outside_rounded_corner()` azonnal `false`-t ad. A framebuffer-menet eleve a `gbitmap_get_data_row_info()` soronkénti `min_x`/`max_x` tartományán fut, így a kör alakú (`GBitmapFormat8BitCircular`) framebufferen is csak a látható pixeleket írja, és a `shadow[]` puffer helyesen viszi tovább a 45°-os árnyékot a változó sorszélességek mellett.
- **Nincs pulzusmérő:** a Round 2-ben nincs HR-szenzor, ezért `HAS_HR_SENSOR = 0` (mint Basalt-on). A C-alapértékek `#if !HAS_HR_SENSOR` alapján, a `companion.js` pedig a `NO_HR_PLATFORMS = ['basalt', 'gabbro']` lista alapján ad `SEC`-et a 2. és `Nothing`-ot a 6. mezőre.
- **`@rebble/clay`:** a `pebble-clay` 1.0.4 natív része csak `aplite`–`emery` platformokra készült, a build `gabbro`-val leáll. A karbantartott `@rebble/clay` 1.1.0 ugyanazt a JS API-t adja (`require('@rebble/clay')`), és `flint`/`gabbro` binárist is tartalmaz.
- **`VIB` információs mező (`INFO_VIBE = 10`), alapértelmezett 3. (jobb felső) mező minden platformon az `ACT` helyett:** `ON`, ha az óra rezeghet, `OFF`, ha a Néma idő (`quiet_time_is_active()`) aktív. A Néma időnek nincs változási eseménye, ezért a mező a percenkénti újrarajzoláskor frissül, és az `app_focus_service` is újrarajzol, amikor az óralap visszakapja a fókuszt (pl. értesítés vagy menü után). Az `N`, `O`, `F` betűk bekerültek a `FONT_INFO_*` fontok `characterRegex`-ébe. Az `ACT` továbbra is választható; a már elmentett beállításokat a változás nem írja felül.
