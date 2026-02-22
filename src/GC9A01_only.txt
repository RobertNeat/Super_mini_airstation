// [env:seeed_xiao_esp32c3]
// platform = espressif32
// board = seeed_xiao_esp32c3
// framework = arduino
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

TFT_eSPI tft = TFT_eSPI();

#define PIN        8
#define NUM_PIXELS 1

Adafruit_NeoPixel strip(NUM_PIXELS, PIN, NEO_GRB);

void setup() {

  strip.begin();
  strip.setBrightness(0);
  strip.setPixelColor(0, strip.Color(0, 0, 255));
  strip.show();

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
}

void loop() {
  tft.setTextColor(0x8c71, TFT_BLACK, true);
  tft.drawString("Hello, World!", 10, 10, 4);
  delay(100);
}