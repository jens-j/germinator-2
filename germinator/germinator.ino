#include <OneWire.h>
#include <DallasTemperature.h>
#include "common.h"
#include "display.h"

// TODO
// - display pid output value if both buttons are pressed
// - log in main loop

// interrupt decimation in ms
#define BTN_PERIOD   20  // ms
#define PID_PERIOD   100 // ms
#define BLINK_PERIOD 200 // ms
#define PID_P        0.2
#define PID_I        0.0005
#define PID_I_MAX    0.8
#define LOG_PERIOD   1   // s
#define SET_PERIOD   3   // s

OneWire oneWire(SENSOR);
DallasTemperature sensors(&oneWire);

volatile int temperature   = 0;
volatile int setpoint      = 28;
volatile int intCounter    = 0;
volatile bool btnUpState   = false;
volatile bool btnDownState = false;
volatile bool btnUpFlag    = false;
volatile bool btnDownFlag  = false;
volatile bool setMode      = false;
volatile float error       = 0.0;
volatile float iSum        = 0.0;
volatile int pwmOutput     = 0;
unsigned long logCounter;
unsigned long setCounter;

SIGNAL(TIMER0_COMPA_vect) {

    bool newBtnUpState;
    bool newBtnDownState;
    float error;
    float output;

    // determine value to display
    int displayValue = setMode ? setpoint : temperature;
    if (btnUpState && btnDownState == true) {
        displayValue = pwmOutput;
    }

    // TDM the 7 segment display
    intCounter & 1 == 1 ? writeDisplay(displayValue % 10, 0) : writeDisplay(displayValue / 10, 1);

    // blink display
    if (setMode && intCounter / BLINK_PERIOD & 1 == 1) {
        blankDisplay();
    }

    // read button states
    if (intCounter % BTN_PERIOD == 0) {
        newBtnUpState = digitalRead(BTN_UP);
        newBtnDownState = digitalRead(BTN_DOWN);
        if (btnUpState == false and newBtnUpState == true) {
            btnUpFlag = true;
        }
        if (btnDownState == false and newBtnDownState == true) {
            btnDownFlag = true;
        }
        btnUpState = newBtnUpState;
        btnDownState = newBtnDownState;
    }

    // update pwm loop
    if (intCounter % PID_PERIOD == 0) {
        sensors.requestTemperatures();
        temperature = int(sensors.getTempCByIndex(0));
        error = setpoint - temperature;
        iSum = constrain(iSum + error * PID_I * PID_PERIOD / 1000, 0, PID_I_MAX);
        output = error * PID_P + iSum;
        pwmOutput = constrain(output * 255, 0, 255) * 5.0 / 12.0;
        analogWrite(HEATER, pwmOutput);
    }

    intCounter++;
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
    Serial.println("# temperature, setpoint, error, i sum, pwm output");

    unsigned long logCounter = millis();
}

void loop() {

    char buffer[100];

    // handle button events
    if (setMode) {
        if (btnUpFlag) {
            btnUpFlag = false;
            setCounter = millis();
            setpoint++;
        }
        if (btnDownFlag) {
            btnDownFlag = false;
            setCounter = millis();
            setpoint--;
        }
        if ((millis() - setCounter) >= SET_PERIOD * 1000) {
            setMode = false;
        }
    } else {
        if (btnUpFlag || btnDownFlag) {
            btnUpFlag = false;
            btnDownFlag = false;
            setCounter = millis();
            setMode = true;
        }
    }

    // log to serial
    if (millis() - logCounter >= LOG_PERIOD * 1000) {
        logCounter = millis();
        sprintf(buffer, "%f,%f,%f,%f,%d", temperature, setpoint, error, iSum, pwmOutput);
        Serial.println(buffer);
    }
}
