#pragma once
#include <TFT_eSPI.h>
#include "ThresholdColor.h"

// ─── ArcGauge ─────────────────────────────────────────────────────────────────
// Draws a partial arc on a TFT_eSprite.
// Handles delta erase/redraw so the sprite never needs a full clear.
//
// Arc geometry: starts at startAngle (default 45°), sweeps up to endAngle
// (default 315°).  Angle 0 = 12 o'clock, clockwise.
//
// Value mapping:
//   minVal → startAngle (empty arc)
//   maxVal → endAngle   (full arc)
class ArcGauge {
public:
    // sprite    : reference to the off-screen sprite
    // cx, cy    : centre of the circle
    // rOuter    : outer radius
    // rInner    : inner radius
    // minVal    : value that maps to arc start
    // maxVal    : value that maps to arc end
    // colors    : ThresholdColor resolver (may be nullptr for fixed colour)
    // fixedColor: used when colors == nullptr
    ArcGauge(TFT_eSprite &sprite,
             int cx, int cy,
             int rOuter, int rInner,
             float minVal, float maxVal,
             const ThresholdColor *colors  = nullptr,
             uint16_t              fixedColor = TFT_WHITE,
             int startAngle = 45, int endAngle = 315);

    // Draw updated value into the sprite (delta only, no full clear).
    // Call sprite.pushSprite() separately after all gauges/texts are updated.
    void draw(float value);

    // Force full redraw on next draw() call (e.g. after screen reinit).
    void invalidate();

private:
    TFT_eSprite &_spr;
    int _cx, _cy;
    int _rOuter, _rInner;
    float _minVal, _maxVal;
    const ThresholdColor *_colors;
    uint16_t _fixedColor;
    int _startAngle, _endAngle;

    int      _prevArcEnd;
    uint16_t _prevColor;
    bool     _firstDraw;

    int   valueToAngle(float value) const;
    uint16_t colorFor(float value) const;
};
