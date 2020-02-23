#include "common.h"

void writeRegister(int data) {

    for (int i = 0; i < 8; i++) {
        digitalWrite(SER, data & 0x80);
        digitalWrite(SRCLK, HIGH);
        digitalWrite(SRCLK, LOW);
        data <<= 1;
    }
    digitalWrite(RCLK, HIGH);
    digitalWrite(RCLK, LOW);
}

void writeDisplay(int digit, int index) {

    int data = index == 0 ? 0x80 : 0x40;

    switch (digit) {
        case 0:
            break;
        case 1:
            data |= 0x39;
            break;
        case 2:
            data |= 0x24;
            break;
        case 3:
            data |= 0x30;
            break;
        case 4:
            data |= 0x19;
            break;
        case 5:
            data |= 0x12;
            break;
        case 6:
            data |= 0x02;
            break;
        case 7:
            data |= 0x38;
            break;
        case 8:
            break;
        case 9:
            data |= 0x10;
    }

    writeRegister(data);

    if (digit < 2 || digit == 7) {
        digitalWrite(SEG_G, HIGH);
    } else {
        digitalWrite(SEG_G, LOW);
    }
}