# Super Mini Airstation

ESP32-C3 based air quality display station with a circular TFT and RGB indicator LED.

## Hardware

| Component | Part | GPIO |
|---|---|---|
| MCU | Seeed XIAO ESP32-C3 | — |
| Display | GC9A01 240×240 round TFT | SCL=1, SDA=2, DC=3, CS=4 |
| LED | NeoPixel (1 pixel) | 8 |
| Sensor | ENS160 + AHT21 *(real driver, swap in)* | I²C |

## Display

Two concentric arcs and three text fields on a 240×240 circular screen:

- **Outer arc** — Humidity (0–100%)
- **Inner arc** — CO₂ (400–1600 ppm), colour changes with level
- **Center text** — Temperature (°C)
- **Upper text** — TVOC (ppb)
- **Lower text** — AQI

Rendering uses a full-screen sprite (off-screen buffer) with delta redraw — only changed arc segments and text regions are touched per frame, avoiding flicker without clearing the screen.

## RGB LED

Bound to the CO₂ arc colour table. Transitions smoothly between colours using per-channel linear stepping (no hard jumps).

## Colour thresholds

A shared `ThresholdColor` table drives both the CO₂ arc and the LED. Bands can be defined in **absolute values** or **percentages** of a configured maximum. Swapping between modes is a single flag in `main.cpp`.

| CO₂ (ppm) | Colour |
|---|---|
| ≤ 600 | Cyan |
| ≤ 800 | Green |
| ≤ 1000 | Yellow |
| ≤ 1200 | Orange |
| > 1200 | Red |

## Value interpolation

Sensor readings are polled on a slow interval (default 10 s). On each new reading, display values transition smoothly to the new target over a configurable number of display frames (`INTERP_STEPS`), eliminating abrupt jumps.

```
SENSOR_INTERVAL_MS = 10000   // how often sensor is read
INTERP_STEPS       = 60      // frames to reach new value (~3 s at 20 fps)
```

## Project structure

```
src/
└── main.cpp                  # entry point only — wiring and tick calls

lib/custom_lib/               # custom modules
    SensorSource              # abstract interface + sine-wave simulator
    ValueInterpolator         # linear interpolation between sensor readings
    ThresholdColor            # colour band table (absolute or %)
    ArcGauge                  # arc widget with delta erase/redraw
    GaugeText                 # text widget with delta erase/redraw
    LedIndicator              # NeoPixel with smooth colour transitions

lib/TFT_eSPI/                 # display driver (external)
```

## Swapping in a real sensor

Replace `SimulatedSensor` with a class that implements `SensorSource`:

```cpp
bool  update()        // return true when a fresh reading is ready
float temperature()
float humidity()
float co2()
float aqi()
float tvoc()
```

One line change in `main.cpp` — nothing else needs to know.

### Next to implement

- initial configuration station mode

- later in client mode connect to the router
- info endpoint after config so the data structure in json could be provided