# Változásnapló — Blokk (Pebble Time & Pebble Time 2)

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
