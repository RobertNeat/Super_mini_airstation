// [env:seeed_xiao_esp32c3]
// platform = espressif32
// board = seeed_xiao_esp32c3
// framework = arduino
// monitor_speed = 115200
// lib_deps = adafruit/Adafruit NeoPixel @ ^1.11.0
//           ;bodmer/TFT_eSPI

//*********[ SETUP ]******[ GC9A01 ]******************[ ESP32-C3 Super Mini ]******************      
// # define TFT_CS 4      //    CS                    GPIO_4
// # define TFT_DC 3      //    DC                    GPIO_3
// # define TFT_MOSI 2    //    SDA                   GPIO_2
// # define TFT_SCLK 1    //    SCL                   GPIO_1
//                              GND                   GND
//                              VCC                   3V3
//*********************************************************************************************

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Adafruit_NeoPixel.h>

#include <Fonts/Custom/Lemon_Milk_Font_20.h>
#include <Fonts/Custom/Lemon_Milk_Font_30.h>
#include <Fonts/Custom/Lemon_Milk_Font_40.h>

// ═══════════════════════════════════════════════════════════════════════════════
//  TECHNIKA BEZMRUGAJĄCEGO ODŚWIEŻANIA (do przeniesienia do biblioteki)
// ───────────────────────────────────────────────────────────────────────────────
//
//  SPRITE (off-screen buffer):
//    Cały ekran rysowany jest do TFT_eSprite w pamięci RAM.
//    Fizyczny wyświetlacz aktualizowany jest jednym wywołaniem pushSprite(),
//    które wysyła gotowy bufor przez SPI – ekran nigdy nie widzi stanu
//    pośredniego (czarnego tła między klatkami).
//    Sprite inicjalizowany jest TYLKO RAZ czarnym kolorem (fillSprite w setup).
//    Potem już nigdy nie jest czyszczony w całości.
//
//  TEKST – delta redraw:
//    updateText() porównuje poprzedni i nowy string (strcmp).
//    Jeśli string się nie zmienił → funkcja nie dotyka ekranu w ogóle.
//    Jeśli się zmienił:
//      1. fillRect() o wymiarach starego tekstu + 2px padding → czyste czarne tło
//         (czarny string zostawiałby artefakty gdy nowy tekst jest węższy)
//      2. drawString() z nowym tekstem w docelowym kolorze
//    Wymaga przechowywania poprzedniego stringa (prevXxx[]).
//
//  ŁUK – delta redraw:
//    updateArc() przyjmuje poprzedni i nowy kąt końcowy łuku.
//      1. Stary łuk rysowany jest czarnym kolorem (maże tylko swój obszar)
//      2. Nowy łuk rysowany jest docelowym kolorem
//    Nie czyści się nic poza faktycznie zmienionym fragmentem łuku.
//    Wymaga przechowywania poprzedniego kąta i koloru (prevXxxArc, prevXxxColor).
//
// ═══════════════════════════════════════════════════════════════════════════════



// ─── Hardware ────────────────────────────────────────────────────────────────
TFT_eSPI    tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

#define PIN        8
#define NUM_PIXELS 1
Adafruit_NeoPixel strip(NUM_PIXELS, PIN, NEO_GRB);

// ─── Sine simulation ─────────────────────────────────────────────────────────
struct SineChannel {
  float base, amp, freq, phase, minVal, maxVal, value;
};

SineChannel chTemp  = {24.0f,  6.0f,  0.31f, 0.00f,  15.0f,  35.0f, 24.0f};
SineChannel chHum   = {55.0f, 30.0f,  0.19f, 1.10f,   0.0f, 100.0f, 55.0f};
SineChannel chCO2   = {800.0f,380.0f, 0.23f, 2.30f, 400.0f,1600.0f,800.0f};
SineChannel chAQI   = {60.0f, 50.0f,  0.17f, 0.80f,   0.0f, 200.0f, 60.0f};
SineChannel chTVOC  = {120.0f,100.0f, 0.27f, 1.70f,   0.0f, 500.0f,120.0f};

void updateSine(SineChannel &ch, float t) {
  ch.value = constrain(ch.base + ch.amp * sinf(ch.freq * t + ch.phase),
                       ch.minVal, ch.maxVal);
}

// ─── Previous arc angles (for delta erase) ───────────────────────────────────
int prevHumArc = 45;
int prevCO2Arc = 45;
uint16_t prevCO2Color = 0x8eff;

// ─── Previous text strings (redraw only if changed) ──────────────────────────
char prevTVOC[24] = "";
char prevTemp[24] = "";
char prevAQI[24]  = "";

// ─── Colour helpers ──────────────────────────────────────────────────────────
uint16_t co2ArcColor(float co2) {
  if      (co2 <= 600)  return 0x8eff;
  else if (co2 <= 800)  return 0x9772;
  else if (co2 <= 1000) return 0xfea0;
  else if (co2 <= 1200) return 0xfd20;
  else                  return 0xfa8a;
}

void co2ToRGB(float co2, uint8_t &r, uint8_t &g, uint8_t &b) {
  if      (co2 <= 600)  { r=  0; g=255; b=128; }
  else if (co2 <= 800)  { r= 50; g=200; b= 50; }
  else if (co2 <= 1000) { r=220; g=200; b=  0; }
  else if (co2 <= 1200) { r=255; g=100; b=  0; }
  else                  { r=255; g=  0; b= 30; }
}

// ─── LED ─────────────────────────────────────────────────────────────────────
uint8_t ledR=0,ledG=255,ledB=128,tgtR=0,tgtG=255,tgtB=128;
#define LED_STEP    4
#define LED_TICK_MS 20

uint8_t stepToward(uint8_t c, uint8_t t, uint8_t s) {
  if (c<t) return (uint8_t)min((int)c+s,(int)t);
  if (c>t) return (uint8_t)max((int)c-s,(int)t);
  return c;
}

// ─── Draw helpers that work directly on the sprite ───────────────────────────
int cx, cy;  // set once in setup

// Erase old arc segment, draw new one – no full clear needed
void updateArc(int r_out, int r_in,
               int oldEnd, int newEnd,
               uint16_t oldColor, uint16_t newColor) {
  // 1. Erase old arc with black
  spr.drawSmoothArc(cx, cy, r_out, r_in, 45, oldEnd, TFT_BLACK, TFT_BLACK, true);
  // 2. Draw new arc
  spr.drawSmoothArc(cx, cy, r_out, r_in, 45, newEnd, newColor, TFT_BLACK, true);
}

// Erase old string with a filled black rect, then draw new string
void updateText(const char *oldStr, const char *newStr,
                int y, const uint8_t *font, uint16_t color) {
  if (strcmp(oldStr, newStr) == 0) return;

  spr.loadFont(font);
  int h = spr.fontHeight();

  // Erase: black rect covering full width of the old string + 2 px padding
  int oldW = spr.textWidth(oldStr);
  int oldX = (spr.width() - oldW) / 2;
  spr.fillRect(oldX - 2, y - 1, oldW + 4, h + 2, TFT_BLACK);

  // Draw new string
  spr.setTextColor(color, TFT_BLACK, true);
  spr.drawString(newStr, (spr.width() - spr.textWidth(newStr)) / 2, y);
}

// ─── Setup ───────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  strip.begin();
  strip.setBrightness(60);
  strip.setPixelColor(0, strip.Color(ledR, ledG, ledB));
  strip.show();

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  spr.createSprite(tft.width(), tft.height());
  spr.setSwapBytes(true);
  spr.fillSprite(TFT_BLACK);   // ← only once, never again

  cx = spr.width()  / 2;
  cy = spr.height() / 2;

  // Initial full draw
  float t = millis() / 1000.0f;
  updateSine(chTemp, t); updateSine(chHum, t); updateSine(chCO2, t);
  updateSine(chAQI,  t); updateSine(chTVOC,t);

  prevHumArc  = constrain((int)(((27.0f/10.0f)*chHum.value)+45.0f), 46, 315);
  prevCO2Arc  = constrain((int)((((chCO2.value-400.0f)*27.0f)/80.0f)+45.0f), 46, 315);
  prevCO2Color = co2ArcColor(chCO2.value);

  spr.drawSmoothArc(cx,cy,120,110,45,prevHumArc,0x04bf,TFT_BLACK,true);
  spr.drawSmoothArc(cx,cy,100, 90,45,prevCO2Arc,prevCO2Color,TFT_BLACK,true);

  snprintf(prevTVOC, sizeof(prevTVOC), "%.0f TVOC", chTVOC.value);
  snprintf(prevTemp, sizeof(prevTemp), "%.1f \xB0""C",  chTemp.value);
  snprintf(prevAQI,  sizeof(prevAQI),  "%.0f AQI",  chAQI.value);

  spr.loadFont(lemonMilkFont20);
  spr.setTextColor(0x8c71, TFT_BLACK, true);
  spr.drawString(prevTVOC, (spr.width()-spr.textWidth(prevTVOC))/2, cy-60);

  spr.loadFont(lemonMilkFont40);
  spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
  spr.drawString(prevTemp, (spr.width()-spr.textWidth(prevTemp))/2, cy-spr.fontHeight()/2);

  spr.loadFont(lemonMilkFont30);
  spr.setTextColor(0xce59, TFT_BLACK, true);
  spr.drawString(prevAQI, (spr.width()-spr.textWidth(prevAQI))/2, cy+40);

  spr.pushSprite(0, 0);
}

// ─── Loop ────────────────────────────────────────────────────────────────────
void loop() {
  static unsigned long lastDraw    = 0;
  static unsigned long lastLedTick = 0;
  static unsigned long lastPrint   = 0;

  unsigned long now = millis();
  float t = now / 1000.0f;

  // ── Sensor + display update @ ~20 fps ──
  if (now - lastDraw >= 50) {
    lastDraw = now;

    updateSine(chTemp, t); updateSine(chHum, t); updateSine(chCO2, t);
    updateSine(chAQI,  t); updateSine(chTVOC,t);

    // ── Arcs (delta only) ──
    int newHumArc  = constrain((int)(((27.0f/10.0f)*chHum.value)+45.0f), 46, 315);
    int newCO2Arc  = constrain((int)((((chCO2.value-400.0f)*27.0f)/80.0f)+45.0f), 46, 315);
    uint16_t newCO2Color = co2ArcColor(chCO2.value);

    updateArc(120, 110, prevHumArc, newHumArc, 0x04bf, 0x04bf);
    updateArc(100,  90, prevCO2Arc, newCO2Arc, prevCO2Color, newCO2Color);

    prevHumArc   = newHumArc;
    prevCO2Arc   = newCO2Arc;
    prevCO2Color = newCO2Color;

    // ── Texts (only if value string changed) ──
    char newTVOC[24], newTemp[24], newAQI[24];
    snprintf(newTVOC, sizeof(newTVOC), "%.0f TVOC", chTVOC.value);
    snprintf(newTemp, sizeof(newTemp), "%.1f \xB0""C",  chTemp.value);
    snprintf(newAQI,  sizeof(newAQI),  "%.0f AQI",  chAQI.value);

    spr.loadFont(lemonMilkFont40);
    int tempY = cy - spr.fontHeight() / 2;

    updateText(prevTVOC, newTVOC, cy - 60, lemonMilkFont20, 0x8c71);
    updateText(prevTemp, newTemp, tempY,   lemonMilkFont40, TFT_WHITE);
    updateText(prevAQI,  newAQI,  cy + 40, lemonMilkFont30, 0xce59);

    strcpy(prevTVOC, newTVOC);
    strcpy(prevTemp, newTemp);
    strcpy(prevAQI,  newAQI);

    // ── Push only changed region would be ideal; pushSprite is still fast ──
    spr.pushSprite(0, 0);

    co2ToRGB(chCO2.value, tgtR, tgtG, tgtB);
  }

  // ── LED smooth transition ──
  if (now - lastLedTick >= LED_TICK_MS) {
    lastLedTick = now;
    ledR = stepToward(ledR, tgtR, LED_STEP);
    ledG = stepToward(ledG, tgtG, LED_STEP);
    ledB = stepToward(ledB, tgtB, LED_STEP);
    strip.setPixelColor(0, strip.Color(ledR, ledG, ledB));
    strip.show();
  }

  // ── Serial debug ──
  if (now - lastPrint >= 1000) {
    lastPrint = now;
    Serial.printf("T=%.1f  H=%.0f  CO2=%.0f  AQI=%.0f  TVOC=%.0f\n",
                  chTemp.value, chHum.value, chCO2.value, chAQI.value, chTVOC.value);
  }
}