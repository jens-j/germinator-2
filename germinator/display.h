#include "common.h"


#define MASK_A   0x20
#define MASK_B   0x10
#define MASK_C   0x40
#define MASK_E   0x01
#define MASK_F   0x02
#define MASK_G   0x80
#define MASK_DG0 0x04
#define MASK_DG1 0x08

const char SEGMENTS[16] = {
    0xFF ^ (MASK_F | MASK_E | MASK_C | MASK_B | MASK_A | MASK_DG0 | MASK_DG1),
    0xFF ^ (MASK_C | MASK_B | MASK_DG0 | MASK_DG1),
    0xFF ^ (MASK_G | MASK_E | MASK_B | MASK_A | MASK_DG0 | MASK_DG1),
    0xFF ^ (MASK_G | MASK_C | MASK_B | MASK_A | MASK_DG0 | MASK_DG1),
    0xFF ^ (MASK_G | MASK_F | MASK_C | MASK_B | MASK_DG0 | MASK_DG1),
    0xFF ^ (MASK_G | MASK_F | MASK_C | MASK_A | MASK_DG0 | MASK_DG1),
    0xFF ^ (MASK_G | MASK_F | MASK_E | MASK_C | MASK_A | MASK_DG0 | MASK_DG1),
    0xFF ^ (MASK_C | MASK_B | MASK_A | MASK_DG0 | MASK_DG1),
    0xFF ^ (MASK_G | MASK_F | MASK_E | MASK_C | MASK_B | MASK_A | MASK_DG0 | MASK_DG1),
    0xFF ^ (MASK_G | MASK_F | MASK_C | MASK_B | MASK_A | MASK_DG0 | MASK_DG1),
    0xFF ^ (MASK_A | MASK_B | MASK_C | MASK_E | MASK_F | MASK_G | MASK_DG0 | MASK_DG1),
    0xFF ^ (MASK_C | MASK_E | MASK_F | MASK_G | MASK_DG0 | MASK_DG1),
    0xFF ^ (MASK_A | MASK_E | MASK_F | MASK_DG0 | MASK_DG1),
    0xFF ^ (MASK_B | MASK_C | MASK_E | MASK_G | MASK_DG0 | MASK_DG1),
    0xFF ^ (MASK_A | MASK_E | MASK_F | MASK_G | MASK_DG0 | MASK_DG1),
    0xFF ^ (MASK_A | MASK_E | MASK_F + MASK_G | MASK_DG0 | MASK_DG1)
};

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

    char data = (index == 0) ? MASK_DG0 : MASK_DG1;

    data |= SEGMENTS[digit];

    writeRegister(data);

    if (digit == 1 || digit == 4 || digit == 7 || digit == 0xA || digit == 0xF) {
        digitalWrite(SEG_D, HIGH);
    } else {
        digitalWrite(SEG_D, LOW);
    }
}

void blankDisplay() {

    writeRegister(0x00);
}
