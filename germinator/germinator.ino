#include <OneWire.h>
#include <DallasTemperature.h>
#include "common.h"
#include "display.h"

OneWire oneWire(SENSOR);  
DallasTemperature sensors(&oneWire);

volatile int temperature = 0;
volatile int intCounter = 0;

SIGNAL(TIMER0_COMPA_vect) {

    if (intCounter++ & 1 == 1) {
        writeDisplay(temperature % 10, 0);
    } else {
        writeDisplay(temperature / 10, 1);
    }
}

void setup() {

    pinMode(SER, OUTPUT);
    pinMode(RCLK, OUTPUT);
    pinMode(SRCLK, OUTPUT);
    pinMode(SEG_D, OUTPUT);
    digitalWrite(SEG_D, LOW);

    noInterrupts(); 
    OCR0A = 0xAF;
    TIMSK0 |= (1 << OCIE0A);
    interrupts(); 

    Serial.begin(115200);
}

void loop() {

    Serial.println("wut");

    sensors.requestTemperatures(); 
    temperature = int(sensors.getTempCByIndex(0));

    Serial.print(sensors.getTempCByIndex(0));
    Serial.print((char) 176); //shows degrees character
    Serial.println("C");

    delay(200);
}
