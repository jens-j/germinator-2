#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 12

OneWire oneWire(ONE_WIRE_BUS);  
DallasTemperature sensors(&oneWire);

void setup () {
    sensors.begin();
    Serial.begin(115200);
}

void loop () {

    sensors.requestTemperatures(); 

    Serial.print("Temperature: ");
    Serial.print(sensors.getTempCByIndex(0));
    Serial.print((char) 176); //shows degrees character
    Serial.println("C");

    delay(500);
}