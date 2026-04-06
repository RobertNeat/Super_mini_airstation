#include "SensorSource.h"
#include <math.h>

SimulatedSensor::SimulatedSensor(uint32_t intervalMs)
    : _chTemp  {24.0f,   6.0f, 0.31f, 0.00f,  15.0f,   35.0f}
    , _chHum   {55.0f,  30.0f, 0.19f, 1.10f,   0.0f,  100.0f}
    , _chCO2   {800.0f,380.0f, 0.23f, 2.30f, 400.0f, 1600.0f}
    , _chAQI   {60.0f,  50.0f, 0.17f, 0.80f,   0.0f,  200.0f}
    , _chTVOC  {120.0f,100.0f, 0.27f, 1.70f,   0.0f,  500.0f}
    , _temp(24.0f), _hum(55.0f), _co2(800.0f), _aqi(60.0f), _tvoc(120.0f)
    , _intervalMs(intervalMs), _lastUpdate(0)
{}

float SimulatedSensor::tick(const Channel &ch, float t) {
    return constrain(ch.base + ch.amp * sinf(ch.freq * t + ch.phase),
                     ch.minVal, ch.maxVal);
}

bool SimulatedSensor::update() {
    uint32_t now = millis();
    if (now - _lastUpdate < _intervalMs) return false;
    _lastUpdate = now;

    float t = now / 1000.0f;
    _temp = tick(_chTemp, t);
    _hum  = tick(_chHum,  t);
    _co2  = tick(_chCO2,  t);
    _aqi  = tick(_chAQI,  t);
    _tvoc = tick(_chTVOC, t);
    return true;
}
