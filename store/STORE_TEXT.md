# Blokk — Pebble / Rebble Appstore Listing

## App Name
`Blokk`

## Category
`Watchfaces`

## Short Description (for store cards / tagline)
Bold condensed block digits with crisp 45° long shadows, 9 color themes & gradients, and customizable health stats for Pebble Time, Time Round, Time 2 & Round 2.

## Full Description (English — ready to paste)

**Blokk** brings the iconic Fitbit watchface (Design: **Zoltan Nagy / Zo Nagy**, Code: **Andor Czafik**) natively to **Pebble Time (`basalt`)**, **Pebble Time Round (`chalk`)**, **Pebble Time 2 (`emery`)** and **Pebble Round 2 (`gabbro`)**.

Large, ultra-legible condensed block digits cast a sharp 45° long black shadow across a vibrant colorful background, framing your daily stats cleanly inside the shadow zone.

### Features
- **Bold Typography & 45° Long Shadows:** Crisp `American Captain` digits, centered circular colon, date header (`MONTH DD WDAY`), and optional bottom-left `AM`/`PM` indicator in 12h mode.
- **9 Color Themes + Diagonal Gradients:** Choose from Red, Orange, Yellow, Green, Turquoise, Blue, Dark Blue, Pink, or Purple — either solid or smooth dithered diagonal gradients.
- **6 Color Selection Modes:**
  - *By the time of day* (default — shifts from red/orange at dawn, yellow in the morning, turquoise/blue in the afternoon, to purple at night)
  - *Choose one color*
  - *Random every day / every hour / every minute*
  - *Random on wrist flick*
- **4 + 4 Customizable Info Fields:** Pick from `STP` (Steps), `BPM` (Heart rate, Pebble Time 2), `BTT` (Battery % or low-battery only ≤30%), `DST` (Distance in km or miles), `CAL` (Calories burned), `ACT` (Active minutes), `SLP` (Sleep hours), `SEC` (Seconds), `VIB` (`ON`, or `OFF` during Quiet Time — shown by default), or `Nothing`.
- **Round Display Support:** Dedicated layouts for the round Pebble Time Round (180×180) and Pebble Round 2 (260×260) screens.
- **Wrist Flick Extra Fields:** Optionally enable a second set of 4 info fields that appears for 20 seconds when you flick your wrist (with optional short vibration respecting Quiet Time).
- **On-Demand Heart Rate (Pebble Time 2):** Optional *Measure heart rate on shake* mode requests 1 Hz sampling for up to 45 seconds until two consistent readings confirm your BPM, blinking the `BPM` label while measuring.
- **Battery Friendly & Offline Settings:** Runs on minute ticks unless `SEC` is visible, and configures completely offline via Clay in the Pebble app.

### Support
Blokk is completely free. If you enjoy it, you can support our work here:
https://czandor.hu/paypal/blokk

---

## Release Notes (`1.1.1`)
- Much faster per-second updates (especially on Pebble Time Round): the unchanged parts of the face are cached and only the seconds are redrawn.
- Watches without a heart-rate sensor now show `ACT` (active minutes) as the default second field; `SEC` moved to the wrist-flick fields. Saved settings are kept.

## Release Notes (`1.1.0`)
- New platforms: Pebble Time Round (`chalk`, 180×180) and Pebble Round 2 (`gabbro`, 260×260) with dedicated round layouts.
- Lower stack usage while drawing (fixes a reboot loop on Pebble Time Round).
- New `VIB` info field: `ON` when the watch can vibrate, `OFF` during Quiet Time. It is now the default top-right field on all watches (replacing `ACT`, which is still available).
- Settings page now uses the maintained `@rebble/clay` package.

## Release Notes (`1.0.1`)
- Added crisp rounded black corners (`R=16` on Pebble Time 2, `R=12` on Pebble Time) matching the original Fitbit Blokk frame.
- Refined info field label and number font sizes (`17px/24px` on Emery, `13px/17px` on Basalt) for cleaner monochrome legibility of `STP`, `BPM`, `BTT`, `ACT`, and `SEC`.

## Release Notes (`1.0.0`)
- Initial native C release for Pebble Time (`basalt`, 144×168) and Pebble Time 2 (`emery`, 200×228).
- 9 color themes with solid and dithered diagonal gradient modes + 6 color selection modes.
- 4 primary + 4 wrist-flick secondary customizable info fields in a 3-row 45° shadow-aligned layout.
- On-demand shake heart rate measurement with blinking `BPM` label on Pebble Time 2.
- Offline Clay settings page.

## Tags / Keywords
`blokk`, `bold`, `shadow`, `minimal`, `color`, `gradient`, `health`, `steps`, `heart rate`, `pebble time 2`, `pebble time round`, `pebble round 2`, `round`, `chalk`, `quiet time`, `emery`, `basalt`, `gabbro`

## Links
- **Support / Donate:** `https://czandor.hu/paypal/blokk`
- **Source / Website:** `https://github.com/czandor/Blokk-Clockface-Pebble`
