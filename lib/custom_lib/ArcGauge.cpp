#include "ArcGauge.h"

ArcGauge::ArcGauge(TFT_eSprite &sprite,
                   int cx, int cy,
                   int rOuter, int rInner,
                   float minVal, float maxVal,
                   const ThresholdColor *colors,
                   uint16_t              fixedColor,
                   int startAngle, int endAngle)
    : _spr(sprite)
    , _cx(cx), _cy(cy)
    , _rOuter(rOuter), _rInner(rInner)
    , _minVal(minVal), _maxVal(maxVal)
    , _colors(colors), _fixedColor(fixedColor)
    , _startAngle(startAngle), _endAngle(endAngle)
    , _prevArcEnd(startAngle), _prevColor(fixedColor)
    , _firstDraw(true)
{}

int ArcGauge::valueToAngle(float value) const {
    float pct = (value - _minVal) / (_maxVal - _minVal);
    pct = constrain(pct, 0.0f, 1.0f);
    int angle = _startAngle + (int)(pct * (_endAngle - _startAngle));
    // Clamp to startAngle+1 so arc always has at least 1° drawn
    return max(angle, _startAngle + 1);
}

uint16_t ArcGauge::colorFor(float value) const {
    return _colors ? _colors->tftColor(value) : _fixedColor;
}

void ArcGauge::draw(float value) {
    int      newEnd   = valueToAngle(value);
    uint16_t newColor = colorFor(value);

    if (!_firstDraw) {
        // Erase old arc
        _spr.drawSmoothArc(_cx, _cy, _rOuter, _rInner,
                           _startAngle, _prevArcEnd,
                           TFT_BLACK, TFT_BLACK, true);
    }

    // Draw new arc
    _spr.drawSmoothArc(_cx, _cy, _rOuter, _rInner,
                       _startAngle, newEnd,
                       newColor, TFT_BLACK, true);

    _prevArcEnd = newEnd;
    _prevColor  = newColor;
    _firstDraw  = false;
}

void ArcGauge::invalidate() {
    _firstDraw = true;
    _prevArcEnd = _startAngle;
}
