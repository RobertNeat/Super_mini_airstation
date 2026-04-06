#pragma once
#include <Adafruit_NeoPixel.h>
#include "ThresholdColor.h"

// ─── LedIndicator ─────────────────────────────────────────────────────────────
// Wraps a single NeoPixel LED with smooth colour transitions.
// Optionally bound to a ThresholdColor table so the target colour
// is set by calling setValueColor(value).
class LedIndicator {
public:
    // pin         : GPIO pin for the NeoPixel data line
    // brightness  : 0–255
    // stepPerTick : how many colour counts to move per tick (smoothing speed)
    LedIndicator(uint8_t pin, uint8_t brightness = 60, uint8_t stepPerTick = 4);

    void begin();

    // Attach a threshold table.  Call before setValueColor().
    void setThresholds(const ThresholdColor *thresholds);

    // Set target colour from a sensor value using the attached threshold table.
    void setValueColor(float value);

    // Set target colour directly (overrides threshold lookup).
    void setTargetColor(uint8_t r, uint8_t g, uint8_t b);

    // Call every LED_TICK_MS milliseconds to step toward target.
    // Returns true if colour actually changed.
    bool tick();

private:
    Adafruit_NeoPixel _strip;
    uint8_t _step;
    const ThresholdColor *_thresholds;

    uint8_t _curR, _curG, _curB;
    uint8_t _tgtR, _tgtG, _tgtB;

    static uint8_t stepToward(uint8_t cur, uint8_t tgt, uint8_t step);
};
