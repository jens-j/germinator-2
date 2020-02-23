
#define SER   4
#define RCLK  6
#define SRCLK 5
#define SEG_G 7

volatile int number = 0;
volatile int intCounter = 0;

SIGNAL(TIMER0_COMPA_vect) {
    //Serial.println("ok");
    if (intCounter++ & 1 == 1) {
        writeDisplay(number % 10, 0);
    } else {
        writeDisplay(number / 10, 1);
    }
}

void writeRegister(int data) {
    Serial.println(data, 16);
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

    int data = index == 1 ? 0x40 : 0x80;

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

void setup() {
    pinMode(SER, OUTPUT);
    pinMode(RCLK, OUTPUT);
    pinMode(SRCLK, OUTPUT);
    pinMode(SEG_G, OUTPUT);
    digitalWrite(SEG_G, LOW);

    noInterrupts();           // disable all interrupts
    // TCCR1A = 0;
    // TCCR1B = 0;
    // TCNT1 = 64911;            // preload timer
    // TCCR1B |= (1 << CS12);    // 256 prescaler 
    // TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt

    OCR0A = 0xAF;
    TIMSK0 |= (1 << OCIE0A);

    interrupts();             // enable all interrupts

    Serial.begin(115200);
}

void loop() {

    for (int i = 0; i < 100; i++) {
        number = i;
        delay(1000);

    }
}
