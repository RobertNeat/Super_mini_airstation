#pragma once
#include <TFT_eSPI.h>

// ─── GaugeText ────────────────────────────────────────────────────────────────
// Renders a single line of text onto a TFT_eSprite using delta redraw:
// only erases and redraws if the formatted string has changed.
//
// The text is horizontally centered at a fixed Y position.
class GaugeText {
public:
    // sprite    : reference to the off-screen sprite
    // y         : top-left Y coordinate of the text row
    // font      : pointer to a VLWB / loadFont-compatible font array
    // color     : RGB565 text colour
    GaugeText(TFT_eSprite &sprite, int y,
              const uint8_t *font, uint16_t color);

    // Render text.  fmt / ... are printf-style.
    // Returns true if the display was changed.
    bool draw(const char *fmt, ...);

    // Adjust Y position (e.g. once font height is known after TFT init).
    void setY(int y) { _y = y; }

    // Force redraw on next draw() call.
    void invalidate();

    // Call once after TFT/sprite is initialised to pre-cache font metrics.
    // Must be called before the first draw().
    void begin();

private:
    TFT_eSprite &_spr;
    int          _y;
    const uint8_t *_font;
    uint16_t     _color;
    char         _prev[32];
    bool         _firstDraw;

    int  _fontH;       // cached font height – loaded once in begin()
    int  _prevW;       // cached pixel width of _prev string
    int  _prevX;       // cached x of _prev string

    // Pointer to a shared "currently loaded font" tracker.
    // When non-null, loadFont is skipped if this font is already active.
    static const uint8_t *s_loadedFont;
};
