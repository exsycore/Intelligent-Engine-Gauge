# Smart Gauge — ESP32-2432S028

A complete, touch-driven automotive smart-gauge UI for the **ESP32-2432S028**
("Cheap Yellow Display"): 2.8" 320x240 TFT, resistive touch, ESP32, landscape.

This phase is **UI + simulation only**. There are no real sensors or ECU
connections. All values come from a mock telemetry provider and the dashboard
always shows a **DEMO** badge and **SIM** source so simulated data can never be
mistaken for live vehicle data.

## What is implemented

- Exactly four dashboard values: one dominant primary gauge and three
  configurable secondary gauges.
- Four complete layout presets: **Classic**, **Sport**, **Minimal**, **Technical**.
- Primary gauge styles: **Arc**, **Bar**, **Numeric**.
- Secondary indicators: **Bar** or **Dots**.
- Configurable metrics for the primary gauge and all three secondary slots.
  Selecting a metric already used elsewhere swaps the two slots, so the same
  metric can never appear twice.
- Settings for accent colour, tick marks, card indicators, backlight brightness,
  mock scenario, and mock update rate.
- Persistent settings in NVS with versioning and CRC validation; corrupt or
  invalid records fall back to defaults.
- Mock scenarios: Engine Idle, Normal Driving, Acceleration, High RPM, Warning.
- Mock rates: Slow (400 ms), Normal (150 ms), Fast (60 ms).
- Dirty-region dashboard redraws; static screens are drawn once on entry.
- Every visible control is wired to an action. The reset action has a
  confirmation screen.

## Hardware assumptions

| Item | Value |
| --- | --- |
| Board | ESP32-2432S028 (CYD), ESP32, 4 MB flash, no PSRAM required |
| Display | 2.8" 320x240 ILI9341 |
| Display bus | HSPI: MOSI 13, MISO 12, SCLK 14, CS 15, DC 2, RST -1, BL 21 |
| Touch | XPT2046 on its own SPI bus: MOSI 32, MISO 39, SCLK 25, CS 33, IRQ 36 |
| Orientation | Landscape, TFT_eSPI rotation 1 |
| Backlight | GPIO 21, active HIGH, 5 kHz PWM for brightness |

The display setup lives in `platformio.ini` (`TFT_eSPI` build flags). Some CYD
clones use different panel controllers or colour order. If colours look
inverted, add `-DTFT_INVERSION_ON=1` (or the appropriate `TFT_RGB_ORDER`
option) to `build_flags`. If the panel stays white, confirm it is an ILI9341
variant rather than an ST7789 unit.

## Build and upload

Prerequisites: Python + PlatformIO Core, or the PlatformIO IDE extension.
Dependencies are installed automatically from `platformio.ini`; the only
library dependency is `bodmer/TFT_eSPI`.

```bash
pio run                 # compile
pio run -t upload       # flash the board
pio device monitor      # serial log at 115200 baud
```

With the VS Code PlatformIO extension, use **Build**, **Upload**, and
**Monitor**. On this machine PlatformIO was invoked as
`~/.platformio/penv/Scripts/pio.exe` because `pio` is not on `PATH`.

Expected boot log:

```text
[settings] source=defaults persisted=yes layout=CLASSIC
[ui] ready: 320x240 landscape, demo=yes
```

`source=nvs` appears on later boots after the first successful save.

## Using the UI

### Dashboard

- Status bar: **DEMO** badge, simulated source, live warning message, and a
  settings button on the right.
- Primary gauge: large value, metric name, unit, arc/bar/numeric visual.
- Three secondary cards (or columns in the Minimal layout) with value, short
  name, unit, and optional progress indicator.
- Tap the settings button at the top-right to open Settings.

### Settings

| Row | What it does |
| --- | --- |
| Dashboard Layout | Choose Classic / Sport / Minimal / Technical with previews; applies immediately |
| Primary Gauge | Choose the primary metric and Arc / Bar / Numeric style |
| Secondary Gauges | Choose metrics for slots 1–3 and Bar / Dots indicators |
| Appearance | Accent colour, tick marks, card indicators |
| Display Settings | Low / Medium / High backlight brightness |
| Simulator | Scenario and update interval |
| Reset Settings | Confirmation screen, then restore factory defaults |

All choices are saved immediately. Use the back arrow at the top-left to return.
The **Minimal** layout intentionally always renders the primary value in the
numeric style, with a thin progress strip.

## Mock telemetry

The simulator is isolated behind `ITelemetryProvider`; the UI only reads that
interface. Values move toward smooth scenario targets rather than jumping
randomly. Fuel level is retained inside the simulator and refills when leaving
the Warning scenario.

| Scenario | Behaviour |
| --- | --- |
| Engine Idle | ~820 RPM, stationary, warm engine, charging voltage |
| Normal Driving | ~65 km/h cruise with gentle speed and RPM variation |
| Acceleration | Repeating pull through the gears, then a short cruise/coast |
| High RPM | Redline load run; also warms coolant and intake air |
| Warning | High temperature, low voltage, low fuel, high RPM; status messages change as thresholds are crossed |

Warning thresholds: RPM ≥ 6800, coolant ≥ 105 °C, battery ≤ 11.8 V,
fuel ≤ 10%, intake ≥ 70 °C, speed ≥ 180 km/h.

The demo starts in Normal Driving unless a different saved scenario exists.
Changing a scenario resets the simulated trajectory and transitions smoothly;
booting directly into Warning seeds visible warning conditions. Fuel level is
kept inside the simulator and refills after leaving Warning.

## Touch calibration

The default raw calibration is for the common CYD panel in landscape:

```cpp
TOUCH_RAW_X_MIN 200
TOUCH_RAW_X_MAX 3700
TOUCH_RAW_Y_MIN 240
TOUCH_RAW_Y_MAX 3800
TOUCH_SWAP_XY   0
TOUCH_INVERT_X  0
TOUCH_INVERT_Y  0
```

These are `#ifndef` macros in `src/hardware/TouchPanel.h`, so they can be
overridden without editing the driver by adding flags to `build_flags`:

```ini
build_flags =
  ...
  -DTOUCH_RAW_X_MIN=210
  -DTOUCH_RAW_X_MAX=3680
  -DTOUCH_INVERT_X=1
```

Symptoms and fixes:

- Touch works but the position is mirrored: set `TOUCH_INVERT_X` or
  `TOUCH_INVERT_Y` to `1`.
- Horizontal and vertical controls are crossed: set `TOUCH_SWAP_XY` to `1`.
- Taps are offset only near the edges: adjust the matching raw min/max pair.
- Nothing responds: verify GPIO 36 is pulled up on the board and that the touch
  controller is powered; this driver gates reads on the IRQ line.

To measure raw values on a specific unit, temporarily print
`touch_.rawX()` / `touch_.rawY()` from `UiApp::loop()` while tapping near each
screen corner, then map those readings to 0..319 and 0..239.

## Persistence

- NVS namespace `smartgauge`, key `cfg`.
- One packed record: magic `0x5A47`, version `1`, payload size, `AppSettings`,
  CRC16/CCITT.
- A bad checksum, wrong size, old version, or out-of-range values are rejected.
  Enum bounds, metric IDs, and duplicate metric assignments are sanitised; the
  safe defaults are then saved.
- `Reset Settings` overwrites the record with defaults.

## Architecture

```text
src/
  main.cpp                  Arduino entry point
  core/AppTypes             enums, metric metadata, formatting, defaults,
                            sanitisation and unique slot assignment
  telemetry/
    Telemetry               shared data model + provider interface + warnings
    MockTelemetry           smooth scenario simulator
  storage/SettingsStorage   NVS record, CRC and version checks
  settings/SettingsManager  validated settings API and persistence
  hardware/TouchPanel       XPT2046 driver on a dedicated SPI bus
  ui/
    UiApp                   ownership, loop, service sync, touch edge detection
    Screen / ScreenManager  screen contract and navigation stack
    Theme                   palette, fonts, rows, toggles, status bar, header
    GaugeRenderer           arcs, ticks, bars, secondary cards
    DashboardScreen         four layouts and dirty-region updates
    screens/…               settings, layout, metric picker, gauge config,
                            appearance, display, mock and reset screens
```

Replacing the mock provider later only requires implementing
`ITelemetryProvider::begin()`, `poll()`, `data()`, `version()`, `isSimulated()`,
and `sourceLabel()`, then giving that provider to `DashboardScreen`. No UI code
reads the mock class directly.

## Rendering and memory

- TFT_eSPI at 40 MHz, no framebuffer, no LVGL, no dynamic containers.
- The dashboard redraws only changed values, arc deltas, indicators, and cards;
  a full dashboard redraw happens on screen entry, layout change, or warning
  colour changes.
- Static settings screens draw only in `enter()`.
- FreeFonts: 9 pt, bold 9 pt, bold 12 pt, bold 18 pt, bold 24 pt.
- Build uses about 22 KB static RAM and 459 KB flash, leaving ample room for a
  real telemetry provider.

## Verification status

Completed in this environment:

- `pio run` succeeds for `env:jczn_2432s028r` with TFT_eSPI 2.5.43 /
  Arduino core 3.2.1.
- Resolution/orientation constants and every screen/button rectangle were
  checked against 320x240 bounds; scroll-free layouts are used throughout.
- Settings sanitisation, unique metric swapping, NVS CRC rejection, mock
  scenario transitions, and dirty-update paths were reviewed.
- The touch driver uses two-sample press/release debouncing and the proven
  XPT2046 command set, with release required between taps.

Not verifiable without the physical device, and therefore not claimed as
passed:

- Actual panel colour/offset and touch calibration on a specific CYD unit.
- Real finger accuracy for the on-screen hit areas.
- Visual appearance, backlight PWM behaviour, and NVS persistence across a
  physical reboot.

The TFT_eSPI build prints a warning that `TOUCH_CS` is not defined. This is
expected: touch is handled by the custom driver on a separate SPI bus, not by
TFT_eSPI.

## Current limitations

- No sensor, ECU, CAN, or serial telemetry integration yet.
- No audio, animation framework, or filesystem assets.
- Single dashboard page; no user-defined metric labels.
- Touch calibration is compile-time, not an on-device wizard.
