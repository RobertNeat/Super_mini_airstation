// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  Super Mini Airstation – main.cpp                                        ║
// ║  Hardware: Seeed XIAO ESP32-C3 + GC9A01 240×240 + NeoPixel (GPIO 8)     ║
// ╚══════════════════════════════════════════════════════════════════════════╝
//
//*********[ SETUP ]******[ GC9A01 ]******************[ ESP32-C3 Super Mini ]***
// # define TFT_CS 4      //    CS                    GPIO_4
// # define TFT_DC 3      //    DC                    GPIO_3
// # define TFT_MOSI 2    //    SDA                   GPIO_2
// # define TFT_SCLK 1    //    SCL                   GPIO_1
//                              GND                   GND
//                              VCC                   3V3
//*******************************************************************************

#include <Arduino.h>
#include <TFT_eSPI.h>

#include <Fonts/Custom/Lemon_Milk_Font_20.h>
#include <Fonts/Custom/Lemon_Milk_Font_30.h>
#include <Fonts/Custom/Lemon_Milk_Font_40.h>

#include "SensorSource.h"
#include "ThresholdColor.h"
#include "ArcGauge.h"
#include "GaugeText.h"
#include "LedIndicator.h"
#include "ValueInterpolator.h"

// ─── Timing ──────────────────────────────────────────────────────────────────
static constexpr uint32_t DISPLAY_TICK_MS  = 50;    // ~20 fps display update
static constexpr uint32_t LED_TICK_MS      = 20;    // 50 fps LED fade
static constexpr uint32_t SERIAL_TICK_MS   = 1000;  // 1 Hz debug print

// Sensor polling interval and how many display frames to spread the transition over.
// At 20 fps, 60 steps = 3 s of smooth animation after each new reading.
static constexpr uint32_t SENSOR_INTERVAL_MS  = 10000; // new reading every 10 s
static constexpr uint16_t INTERP_STEPS        = 60;    // frames to reach target

// ─── Hardware ────────────────────────────────────────────────────────────────
TFT_eSPI    tft;
TFT_eSprite spr(&tft);

// ─── CO2 colour thresholds ───────────────────────────────────────────────────
// Absolute ppm values.  Both arc and LED share the same table.
// Swap usePercent/maxAbsValue to use percentage bands instead.
static const ThresholdEntry co2Thresholds[] = {
    //  upTo ppm   arc RGB565   LED  R    G    B
    {  600,  0x8eff,   0,  255, 128 },  // cyan
    {  800,  0x9772,  50,  200,  50 },  // green
    { 1000,  0xfea0, 220,  200,   0 },  // yellow
    { 1200,  0xfd20, 255,  100,   0 },  // orange
    { 9999,  0xfa8a, 255,    0,  30 },  // red
};
static const ThresholdColor co2Colors(co2Thresholds, 5,
                                      /*usePercent=*/false, /*maxAbs=*/1600.0f);

// ─── Sensor (swap SimulatedSensor for a real driver later) ───────────────────
SimulatedSensor sensor(SENSOR_INTERVAL_MS);

// ─── Interpolators (one per displayed channel) ───────────────────────────────
ValueInterpolator iTemp (24.0f,  INTERP_STEPS);
ValueInterpolator iHum  (55.0f,  INTERP_STEPS);
ValueInterpolator iCO2  (800.0f, INTERP_STEPS);
ValueInterpolator iAQI  (60.0f,  INTERP_STEPS);
ValueInterpolator iTVOC (120.0f, INTERP_STEPS);

// ─── Display layout (centre = 120,120 on 240×240 circle) ────────────────────
static constexpr int CX = 120, CY = 120;

//                           sprite  cx   cy  rOut rIn  min     max    colors      fixedColor
ArcGauge humidityArc (spr, CX, CY, 120, 110,  0.0f, 100.0f, nullptr,    0x04bf);
ArcGauge co2Arc      (spr, CX, CY, 100,  90,400.0f,1600.0f, &co2Colors, 0x8eff);

//                       sprite  y          font             color
GaugeText tvocText (spr, CY - 60, lemonMilkFont20, 0x8c71);
GaugeText tempText (spr, 0,       lemonMilkFont40, TFT_WHITE);  // Y set in setup
GaugeText aqiText  (spr, CY + 40, lemonMilkFont30, 0xce59);

// ─── RGB LED (GPIO 8, bound to CO2 thresholds) ───────────────────────────────
LedIndicator led(/*pin=*/8, /*brightness=*/60, /*stepPerTick=*/4);

// ═════════════════════════════════════════════════════════════════════════════
void setup() {
    Serial.begin(115200);

    led.begin();
    led.setThresholds(&co2Colors);

    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);

    spr.createSprite(tft.width(), tft.height());
    spr.setSwapBytes(true);
    spr.fillSprite(TFT_BLACK);  // only once — never cleared again

    // Pre-cache font metrics (one flash read each)
    tvocText.begin();
    aqiText.begin();
    tempText.begin();
    tempText.setY(CY - spr.fontHeight() / 2);

    // Force first sensor read immediately
    sensor.update();
    iTemp.setTarget(sensor.temperature());
    iHum .setTarget(sensor.humidity());
    iCO2 .setTarget(sensor.co2());
    iAQI .setTarget(sensor.aqi());
    iTVOC.setTarget(sensor.tvoc());

    // Initial draw at starting values
    humidityArc.draw(iHum.value());
    co2Arc.draw(iCO2.value());
    tvocText.draw("%.0f TVOC", iTVOC.value());
    tempText.draw("%.1f \xB0""C",  iTemp.value());
    aqiText .draw("%.0f AQI",  iAQI.value());

    spr.pushSprite(0, 0);
}

// ═════════════════════════════════════════════════════════════════════════════
void loop() {
    static uint32_t lastDraw  = 0;
    static uint32_t lastLed   = 0;
    static uint32_t lastPrint = 0;

    uint32_t now = millis();

    // ── Sensor poll (slow) — only sets new interpolation targets ────────────
    if (sensor.update()) {
        iTemp.setTarget(sensor.temperature());
        iHum .setTarget(sensor.humidity());
        iCO2 .setTarget(sensor.co2());
        iAQI .setTarget(sensor.aqi());
        iTVOC.setTarget(sensor.tvoc());
    }

    // ── Display tick ~20 fps ─────────────────────────────────────────────────
    if (now - lastDraw >= DISPLAY_TICK_MS) {
        lastDraw = now;

        iTemp.tick(); iHum.tick(); iCO2.tick(); iAQI.tick(); iTVOC.tick();

        humidityArc.draw(iHum.value());
        co2Arc.draw(iCO2.value());
        tvocText.draw("%.0f TVOC", iTVOC.value());
        tempText.draw("%.1f \xB0""C",  iTemp.value());
        aqiText .draw("%.0f AQI",  iAQI.value());

        spr.pushSprite(0, 0);

        led.setValueColor(iCO2.value());
    }

    // ── LED smooth fade 50 fps ───────────────────────────────────────────────
    if (now - lastLed >= LED_TICK_MS) {
        lastLed = now;
        led.tick();
    }

    // ── Serial debug 1 Hz ───────────────────────────────────────────────────
    if (now - lastPrint >= SERIAL_TICK_MS) {
        lastPrint = now;
        Serial.printf("T=%.1f  H=%.0f  CO2=%.0f  AQI=%.0f  TVOC=%.0f\n",
                      iTemp.value(), iHum.value(),
                      iCO2.value(), iAQI.value(), iTVOC.value());
    }
}
