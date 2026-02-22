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



// ENS160 + AHT21 sensor (standard reading mode)
// Connections from sensors to esp_32:
// VIN - to 3V3 esp_32 pin
// 3V3 - DON't CONNECT (weak power)
// GND - to GND esp_32 pin
// SCL - to GPIO_6
// SDA - to GPIO_7


#include <Arduino.h>
#include <Wire.h>
#include <AirSensor.h>
#include <TFT_eSPI.h>
#include <Adafruit_NeoPixel.h>
#include <Fonts/Custom/Lemon_Milk_Font_10.h>
#include <Fonts/Custom/Lemon_Milk_Font_20.h>
#include <Fonts/Custom/Lemon_Milk_Font_30.h>
#include <Fonts/Custom/Lemon_Milk_Font_40.h>

TFT_eSPI tft = TFT_eSPI();
AirSensor airSensor;

String temp_text = "";
String aqi_text = "";
String tvoc_text = "";
float humidity_value = 0.0;
float co2_value = 0.0;

int humidity_arc = 0;
int co2_arc = 0;
uint16_t co2_arc_color = 0x8eff;

#define PIN        8
#define NUM_PIXELS 1

Adafruit_NeoPixel strip(NUM_PIXELS, PIN, NEO_GRB);

void setup() {
  Serial.begin(115200);//460800
  delay(1000); // Daj czas na stabilizację Serial
  Serial.println("\n\nSetup");

  // Initialize I2C on GPIO7 (SDA) and GPIO6 (SCL)
  Wire.begin(7, 6);
  Serial.println("I2C initialized on GPIO7 (SDA), GPIO6 (SCL)");

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  Serial.println("#-----/ENS160 (Indoor Air Quality) + AHT21 (temperature, humidity)/-----#");
  if (!airSensor.begin()) {
      Serial.println("Failed to initialize sensors!");
      while (1) delay(10);
  }

  strip.begin();
  strip.setBrightness(32);
  strip.show();
}

void loop() {

  Serial.println("Loop");
  airSensor.update();

  //1. GET READOUTS
    //airSensor.getReadout();//for debugging
    temp_text = airSensor.getTemperature();
    temp_text += " °C";
    aqi_text = airSensor.getAQI();
    aqi_text += " AQI";
    tvoc_text = airSensor.getTVOC();
    tvoc_text+= " TVOC";
    humidity_value = airSensor.getHumidity();
    co2_value = airSensor.getCO2();

    //2. PRINT TEXT DATA
    tft.loadFont(lemonMilkFont20);
    tft.setTextColor(0x8c71, TFT_BLACK, true);
    tft.drawString(tvoc_text,(tft.width()-tft.textWidth(tvoc_text))/2,((tft.height()-tft.fontHeight())/2)-50);

    tft.loadFont(lemonMilkFont40);
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.drawString(temp_text,(tft.width()-tft.textWidth(temp_text))/2,(tft.height()-tft.fontHeight())/2);

    tft.loadFont(lemonMilkFont30);
    tft.setTextColor(0xce59, TFT_BLACK, true);
    tft.drawString(aqi_text,(tft.width()-tft.textWidth(aqi_text))/2,((tft.height()-tft.fontHeight())/2)+50);

    //3. DRAW ARCS
    //humidity (0-100 value to 45-315) <-- outer ring
    humidity_arc = ((27.0/10.0)*humidity_value)+45.0;
    tft.drawSmoothArc(tft.width()/2, tft.height()/2, 120, 110, 45, humidity_arc, 0x04bf, TFT_BLACK, true);
      
    //co2 value <-- inner ring
    co2_arc = (((co2_value-400.0)*27.0)/80)+45;
        Serial.print("CO2:");
        Serial.println(co2_arc);
          if(co2_value<=600){co2_arc_color = 0x8eff;} //change color for specific tresholds (excellent - target)
          else if(co2_value<=800){co2_arc_color = 0x9772;}// (good - sufficient ventillation recommended)
          else if(co2_value<=1000){co2_arc_color = 0xfea0;}// (moderate - increased ventillation recommended)
          else if (co2_value<=1200){co2_arc_color = 0xfd20;}// (poor - intensified ventillation recommended)
          else{co2_arc_color = 0xfa8a;}// (unhealthy - use only if unavoidable)
    tft.drawSmoothArc(tft.width()/2, tft.height()/2, 100, 90, 45, co2_arc, co2_arc_color, TFT_BLACK, true);//315

    //debugging
    Serial.println(temp_text);
    Serial.println(aqi_text);
    Serial.println(tvoc_text);
    Serial.println(humidity_value);
    Serial.println(co2_value);

    strip.setPixelColor(0,co2_value);
    strip.show();


    delay(10000);
    tft.fillScreen(TFT_BLACK);  //tempopary fix for updating text and arcs (maybe there is a function of I will use the bigger black shape)
    
}