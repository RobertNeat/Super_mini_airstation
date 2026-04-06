#pragma once
#include <Arduino.h>

// ─── Single threshold entry ───────────────────────────────────────────────────
// upTo: upper bound for this band.
//   - If usePercent == true  → 0.0–100.0 (% of maxAbsValue)
//   - If usePercent == false → absolute value
// tftColor : 16-bit RGB565 colour for the arc / display
// r, g, b  : 8-bit RGB for the LED
struct ThresholdEntry {
    float   upTo;
    uint16_t tftColor;
    uint8_t  r, g, b;
};

// ─── Colour resolver for a list of thresholds ────────────────────────────────
class ThresholdColor {
public:
    // entries    : pointer to array of ThresholdEntry (ascending upTo order)
    // count      : number of entries
    // usePercent : true → upTo values are % of maxAbsValue
    // maxAbsValue: ignored when usePercent == false
    ThresholdColor(const ThresholdEntry *entries, uint8_t count,
                   bool usePercent = false, float maxAbsValue = 100.0f);

    // Returns the TFT 16-bit colour matching value
    uint16_t tftColor(float value) const;

    // Fills r/g/b with the LED colour matching value
    void ledColor(float value, uint8_t &r, uint8_t &g, uint8_t &b) const;

private:
    const ThresholdEntry *_entries;
    uint8_t  _count;
    bool     _usePercent;
    float    _maxAbsValue;

    uint8_t resolve(float value) const;  // returns matching entry index
};
