#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "common.h"
#include "display.h"

// TODO
// - display pid output value if both buttons are pressed
// - log in main loop

// interrupt decimation in ms
#define BTN_PERIOD   20  // ms
#define BLINK_PERIOD 100 // ms
#define PID_P        0.2
#define PID_I        1E-4
#define PID_I_MAX    0.8
#define LOG_PERIOD   1   // s
#define SET_PERIOD   1.5 // s


OneWire oneWire(SENSOR);
DallasTemperature sensors(&oneWire);

volatile int intCounter    = 0;
volatile bool btnUpState   = false;
volatile bool btnDownState = false;
volatile bool btnUpFlag    = false;
volatile bool btnDownFlag  = false;
volatile unsigned long isr_t0 = 0;
volatile unsigned long isr_dt = 0;
volatile bool updateEeprom = false;

int setpoint               = 28;
bool setMode               = false;
float iSum                 = 0.0;
float temperature          = 0;
int pwmOutput              = 0;
unsigned long loop_t0 = 0;
unsigned long loop_dt = 0;

unsigned long logCounter;
unsigned long setCounter;
unsigned long pidCounter;


SIGNAL(TIMER0_COMPA_vect) {

    bool newBtnUpState;
    bool newBtnDownState;
    int displayValue;

    unsigned long isr_t1 = millis();
    isr_dt = isr_t1 - isr_t0;
    isr_t0 = isr_t1;

    // blink display
    if (setMode && intCounter / BLINK_PERIOD & 1 == 1) {
        blankDisplay();
    } else if ((btnUpState && btnDownState == true)) {
        (intCounter & 1 == 1) ? // TDM the 7 segment display
            writeDisplay((pwmOutput) % 16, 0) :
            writeDisplay((pwmOutput) / 16, 1);
    } else {
        displayValue = setMode ? setpoint : temperature + 0.5; // rounding
        (intCounter & 1 == 1) ? // TDM the 7 segment display
            writeDisplay((displayValue) % 10, 0) :
            writeDisplay((displayValue) / 10, 1);
    }

    // read button states
    if (intCounter % BTN_PERIOD == 0) {
        newBtnUpState = digitalRead(BTN_UP);
        newBtnDownState = digitalRead(BTN_DOWN);
        if (btnUpState == false && newBtnUpState == true) {
            btnUpFlag = true;
        }
        if (btnDownState == false && newBtnDownState == true) {
            btnDownFlag = true;
        }
        btnUpState = newBtnUpState;
        btnDownState = newBtnDownState;
    }

    // handle button events
    if (btnUpFlag) {
        btnUpFlag = false;
        setMode = true;
        setpoint++;
        intCounter = 0;
        setCounter = millis();
    } else if (btnDownFlag) {
        btnDownFlag = false;
        setMode = true;
        setpoint--;
        intCounter = 0;
        setCounter = millis();
    }

    // exit blink mode
    if ((millis() - setCounter) >= SET_PERIOD * 1000) {
        setMode = false;
        updateEeprom = true;
    }

    intCounter++;
}

void setup() {

    pinMode(SER, OUTPUT);
    pinMode(RCLK, OUTPUT);
    pinMode(SRCLK, OUTPUT);
    pinMode(SEG_D, OUTPUT);
    digitalWrite(SEG_D, HIGH);

    logCounter = millis();
    pidCounter = millis();
    setpoint = EEPROM.read(0);

    // set up 1ms interrupt
    noInterrupts();
    OCR0A = 0xAF;
    TIMSK0 |= (1 << OCIE0A);
    interrupts();

    Serial.begin(115200);
}

void loop() {

    unsigned long newPidCounter;
    unsigned long dt;
    float error;
    float output;
    char buffer[200];

    unsigned long loop_t1 = millis();
    loop_dt = loop_t1 - loop_t0;
    loop_t0 = loop_t1;

    // this takes 700 ms!
    sensors.requestTemperatures();
    temperature = sensors.getTempCByIndex(0);
    // temperature = 28.0;

    // update pid loop
    newPidCounter = millis();
    dt = newPidCounter - pidCounter;
    pidCounter = newPidCounter;
    error = setpoint - temperature;
    iSum = constrain(iSum + error * PID_I * dt / 1000, 0, PID_I_MAX);
    output = constrain(error * PID_P + iSum, 0, 1);
    pwmOutput = output * (255 * 5.0 / 12.0); // the heaters are rated for 5V
    analogWrite(HEATER, pwmOutput);

    if (updateEeprom) {
        EEPROM.write(0, setpoint);
        updateEeprom = false;
    }

    // log to serial
    if (millis() - logCounter >= LOG_PERIOD * 1000) {
        logCounter = millis();

        Serial.print("\ntemperature: ");
        Serial.println(temperature);
        Serial.print("setpoint: ");
        Serial.println(setpoint);
        Serial.print("iSum: ");
        Serial.println(iSum);
        Serial.print("pwmOutput: ");
        Serial.println(pwmOutput);
        Serial.print("isr_dt: ");
        Serial.println(isr_dt);
        Serial.print("loop_dt: ");
        Serial.println(loop_dt);
    }
}
