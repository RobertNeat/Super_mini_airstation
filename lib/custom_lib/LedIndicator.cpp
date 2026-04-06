#include "LedIndicator.h"

LedIndicator::LedIndicator(uint8_t pin, uint8_t brightness, uint8_t stepPerTick)
    : _strip(1, pin, NEO_GRB + NEO_KHZ800)
    , _step(stepPerTick)
    , _thresholds(nullptr)
    , _curR(0), _curG(255), _curB(128)
    , _tgtR(0), _tgtG(255), _tgtB(128)
{
    (void)brightness;  // stored via setBrightness in begin()
    _strip.setBrightness(brightness);
}

void LedIndicator::begin() {
    _strip.begin();
    _strip.setPixelColor(0, _strip.Color(_curR, _curG, _curB));
    _strip.show();
}

void LedIndicator::setThresholds(const ThresholdColor *thresholds) {
    _thresholds = thresholds;
}

void LedIndicator::setValueColor(float value) {
    if (!_thresholds) return;
    _thresholds->ledColor(value, _tgtR, _tgtG, _tgtB);
}

void LedIndicator::setTargetColor(uint8_t r, uint8_t g, uint8_t b) {
    _tgtR = r; _tgtG = g; _tgtB = b;
}

uint8_t LedIndicator::stepToward(uint8_t cur, uint8_t tgt, uint8_t step) {
    if (cur < tgt) return (uint8_t)min((int)cur + step, (int)tgt);
    if (cur > tgt) return (uint8_t)max((int)cur - step, (int)tgt);
    return cur;
}

bool LedIndicator::tick() {
    uint8_t r = stepToward(_curR, _tgtR, _step);
    uint8_t g = stepToward(_curG, _tgtG, _step);
    uint8_t b = stepToward(_curB, _tgtB, _step);

    if (r == _curR && g == _curG && b == _curB) return false;

    _curR = r; _curG = g; _curB = b;
    _strip.setPixelColor(0, _strip.Color(r, g, b));
    _strip.show();
    return true;
}
