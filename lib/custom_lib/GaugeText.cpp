#include "GaugeText.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

const uint8_t *GaugeText::s_loadedFont = nullptr;

GaugeText::GaugeText(TFT_eSprite &sprite, int y,
                     const uint8_t *font, uint16_t color)
    : _spr(sprite), _y(y), _font(font), _color(color), _firstDraw(true)
    , _fontH(0), _prevW(0), _prevX(0)
{
    _prev[0] = '\0';
}

void GaugeText::begin() {
    _spr.loadFont(_font);
    s_loadedFont = _font;
    _fontH = _spr.fontHeight();
}

bool GaugeText::draw(const char *fmt, ...) {
    char buf[32];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (!_firstDraw && strcmp(_prev, buf) == 0) return false;

    // Only reload from flash if a different font is currently active.
    if (s_loadedFont != _font) {
        _spr.loadFont(_font);
        s_loadedFont = _font;
    }
    _spr.setTextColor(_color, TFT_BLACK, true);

    if (!_firstDraw) {
        _spr.fillRect(_prevX - 2, _y - 1, _prevW + 4, _fontH + 2, TFT_BLACK);
    }

    int newW = _spr.textWidth(buf);
    int newX = (_spr.width() - newW) / 2;
    _spr.drawString(buf, newX, _y);

    // Cache for next erase
    _prevW = newW;
    _prevX = newX;
    strncpy(_prev, buf, sizeof(_prev) - 1);
    _firstDraw = false;
    return true;
}

void GaugeText::invalidate() {
    _firstDraw = true;
    _prev[0] = '\0';
    _prevW = 0;
    _prevX = 0;
}
