//This is base library to take readouts for the ENS160 + AHT21 sensor
#ifndef AIRSENSOR_H
#define AIRSENSOR_H

#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include "ScioSense_ENS160.h"

class AirSensor {
public:
    AirSensor();
    bool begin();
    void update();
    
    float getTemperature() const;
    float getHumidity() const;
    int getAQI() const;
    int getTVOC() const;
    int getCO2() const;
    void getReadout();

private:
    Adafruit_AHTX0 aht;
    ScioSense_ENS160 ens160;

    float temperature;
    float humidity;
    int aqi;
    int tvoc;
    int co2;
};

#endif // AIRSENSOR_H
