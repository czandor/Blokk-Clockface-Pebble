# Fejlesztői átadás — Blokk (Pebble Time 2)

## Mi hol van

| Fájl | Feladat |
| --- | --- |
| `pebble/src/c/main.c` | Emery (200×228) natív C rajzolás, egymenetes 45°-os hosszú árnyék, 4×4 Bayer ditherelt gradiens, Pebble Health, rázás és beállításkezelés. |
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

Emulátoros telepítés (`NineTimeSDK` WSL-ben):

```bash
export PATH=/opt/ninetime-cli/bin:/usr/local/bin:/usr/bin:/bin
cd /mnt/c/andor/Pebble/Blokk/pebble
pebble install --emulator emery ../dist/Blokk-1.0.0.pbw
```

Fizikai órára (telefonon bekapcsolt Developer Connection mellett):

```bash
pebble install --phone TELEFON_IP ../dist/Blokk-1.0.0.pbw
```

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
