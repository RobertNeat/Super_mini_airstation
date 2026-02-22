//ENS160 datasheet: https://sklep.msalamon.pl/download/38350/?tmstv=1681914627
//AHT21 datasheet: https://sklep.msalamon.pl/download/38353/?tmstv=1681914650

#include <Arduino.h>
#include <AirSensor.h>

AirSensor airSensor;

void setup() {
    Serial.begin(115200);
    Serial.println("#-----/ENS160 (Indoor Air Quality) + AHT21 (temperature, humidity)/-----#");

    if (!airSensor.begin()) {
        Serial.println("Failed to initialize sensors!");
        while (1) delay(10);
    }
}

void loop() {
    airSensor.update();

    /*
    Serial.println("### Sensor Readings ###");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" % rH");

    Serial.print("Air Quality Index (AQI): ");
    Serial.println(aqi);

    Serial.print("Total Volatile Organic Compounds (TVOC): ");
    Serial.print(tvoc);
    Serial.println(" ppb");

    Serial.print("Equivalent CO2 (eCO2): ");
    Serial.print(co2);
    Serial.println(" ppm");
    */
   
    airSensor.getReadout();

    delay(1000);
}
