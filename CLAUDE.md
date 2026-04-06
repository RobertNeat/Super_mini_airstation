# CLAUDE.md — Super Mini Airstation

## Project

PlatformIO / Arduino framework project for Seeed XIAO ESP32-C3.
Target: `seeed_xiao_esp32c3`. Build with PlatformIO (`pio run`).

## File layout

```
src/main.cpp              — entry point only; no logic beyond wiring and tick calls
lib/custom_lib/           — all custom C++ modules (headers + sources co-located)
lib/TFT_eSPI/             — external display driver (do not modify)
lib/Adafruit_BusIO/       — external (do not modify)
```

## Custom modules

| File | Responsibility |
|---|---|
| `SensorSource` | Abstract `SensorSource` interface + `SimulatedSensor` implementation |
| `ValueInterpolator` | Smoothly steps a float from current to target over N frames |
| `ThresholdColor` | Maps a value to RGB565 / LED RGB via a threshold band table |
| `ArcGauge` | Draws a partial arc on a `TFT_eSprite` with delta erase/redraw |
| `GaugeText` | Draws centred text on a `TFT_eSprite` with delta erase/redraw |
| `LedIndicator` | NeoPixel wrapper with smooth per-channel colour transitions |

## Key conventions

- **No full sprite clear** — `fillSprite` is called once in `setup()`. All updates are delta only (erase old region, draw new region).
- **Font loading** — `GaugeText::begin()` must be called after sprite creation to pre-cache font height. `GaugeText` tracks the last loaded font via a static pointer to avoid redundant flash reads.
- **Sensor interface** — `SensorSource::update()` returns `true` only when a fresh reading is available. Display code must not assume values change every frame.
- **Interpolation** — sensor targets feed `ValueInterpolator`; display always reads interpolated values, never raw sensor values directly.
- **ThresholdColor** — shared between arc and LED for the same channel. Supports absolute values or percentage bands (`usePercent` flag + `maxAbsValue`).

## Adding a real sensor

Implement `SensorSource` and replace `SimulatedSensor sensor` in `main.cpp`. No other files need changes.

## Timing constants (main.cpp)

| Constant | Default | Notes |
|---|---|---|
| `DISPLAY_TICK_MS` | 50 | ~20 fps |
| `LED_TICK_MS` | 20 | 50 fps LED fade |
| `SENSOR_INTERVAL_MS` | 10000 | Sensor poll interval |
| `INTERP_STEPS` | 60 | Frames to reach new target (~3 s at 20 fps) |
