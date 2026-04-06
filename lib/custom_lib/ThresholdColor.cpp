#include "ThresholdColor.h"

ThresholdColor::ThresholdColor(const ThresholdEntry *entries, uint8_t count,
                               bool usePercent, float maxAbsValue)
    : _entries(entries), _count(count)
    , _usePercent(usePercent), _maxAbsValue(maxAbsValue)
{}

uint8_t ThresholdColor::resolve(float value) const {
    float v = _usePercent ? (value / _maxAbsValue * 100.0f) : value;
    for (uint8_t i = 0; i < _count - 1; i++) {
        if (v <= _entries[i].upTo) return i;
    }
    return _count - 1;
}

uint16_t ThresholdColor::tftColor(float value) const {
    return _entries[resolve(value)].tftColor;
}

void ThresholdColor::ledColor(float value, uint8_t &r, uint8_t &g, uint8_t &b) const {
    const auto &e = _entries[resolve(value)];
    r = e.r; g = e.g; b = e.b;
}
