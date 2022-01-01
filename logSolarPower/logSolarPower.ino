/*
 * vim: tabstop=4 shiftwidth=4 expandtab syntax=c
 *
 * This solution reads voltage from sensor once-per-minute,
 * reads time from RTC once per hour to prevent local drift
 * and writes time and converted reading to current to the file
 *
 * Two resistors, 3R3 and 33R in series. 3.239 32.80
 * Resistance = 36.039
 * Resistor divider: 7400R and 1000R
 */


#include <stdio.h>
#include <string.h>

#include <SPI.h>
#include <SD.h>

#include <Wire.h>
#include "RTClib.h"

#include "U8glib.h"

#define MAX_STRING 40

#define CARD_SELECT_PORT 10
#define ERROR_LED_PORT 13

//#define TESTING_TIMER 1

// seconds between sensor reads
#ifdef TESTING_TIMER
#define READ_PERIOD 1
#else
#define READ_PERIOD 60
#endif

// Counts number of interrupts to get one minute reading
volatile uint8_t loopCount = 0;
volatile uint8_t triggerLogging = 0;

uint16_t currentReading;
volatile uint8_t loggingFail = 0;

#ifdef TESTING_TIMER
volatile uint8_t secondCounter = 0;
#endif

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);

File dataFile;

char loggingString[MAX_STRING];

RTC_DS1307 rtc;
DateTime now;

volatile uint8_t errorLedStatus = 0;
void triggerErrorLed() {
    if (!errorLedStatus) {
        errorLedStatus = 1;
        digitalWrite(ERROR_LED_PORT, HIGH);
    } else {
        disableErrorLed();
    }
}

void disableErrorLed() {
    errorLedStatus = 0;
    digitalWrite(ERROR_LED_PORT, LOW);
}


void startISR() {
    cli();
    TCCR1A = 0;
    TCCR1B = 0; // Stop timer so we can modify it
    TCCR1B |= _BV(WGM12);; // Configure timer 1 for CTC mode
#ifdef TESTING_TIMER
    OCR1A   = 15625/5 - 1; // Set 16-bit CTC compare value to 5Hz at 16MHz AVR clock, with a prescaler of 1024
#else
    OCR1A   = 15625 - 1; // Set 16-bit CTC compare value to 1Hz at 16MHz AVR clock, with a prescaler of 1024
#endif
    TCCR1B |= (1 << CS10 | 1 << CS12); // Set up timer at Fcpu/1024
    TIMSK1 |= (1 << OCIE1A); // | (1 << OCIE1B); // Enable CTC interrupt
    sei(); //  Enable global interrupts
}


ISR(TIMER1_COMPA_vect) {
#ifdef TESTING_TIMER
    secondCounter++;
#endif
    if (++loopCount >= READ_PERIOD) {
        loopCount = 0;
        triggerLogging++;
        if (loggingFail) {
            triggerErrorLed();
        }
    }
}


void setRTCCompileTime(RTC_DS1307 *rtc) {
    rtc->adjust(DateTime(F(__DATE__), F(__TIME__)));
}


void initializeRTC(RTC_DS1307 * rtc) {
    while (!rtc->begin()) {
        Serial.println(F("Couldn't find RTC"));
        triggerErrorLed();
        delay(1000);
    }

    while (!rtc->isrunning()) {
        Serial.println(F("RTC is NOT running!"));
        triggerErrorLed();
        delay(1000);
    }

    Serial.println(F("RTC Initialized."));

    disableErrorLed();
}


void initializeSdCard() {
    Serial.print(F("Initializing SD card... "));

    // see if the card is present and can be initialized
    while (!SD.begin(CARD_SELECT_PORT)) {
        Serial.println(F("Card failed, or not present"));
        triggerErrorLed();
        delay(1000);
    }

    disableErrorLed();

    Serial.println(F("Card initialized."));
}


void initializeDisplay(U8GLIB_SSD1306_128X64 *u8g) {
    // assign default color value
    u8g->setColorIndex(1); // pixel on

    u8g->firstPage();
    u8g->setFont(u8g_font_unifont);
    do {
        u8g->drawStr(0, 15, "Started");
    } while (u8g->nextPage());
    Serial.println(F("Display initialized!"));
}


void displayWrite(U8GLIB_SSD1306_128X64 * u8g, char * displayString, uint8_t column) {
      u8g->drawStr(0, column * 12, displayString);
}


void setup() {
    Serial.begin(115200);
    pinMode(ERROR_LED_PORT, OUTPUT);
    disableErrorLed();
    initializeSdCard();
    initializeRTC(&rtc);
    initializeDisplay(&u8g);
    startISR();
    Serial.println(F("Running.."));
    triggerLogging = 1;
}


void writeDataToFile(const char * fileName, char * str) {
    dataFile = SD.open(fileName, FILE_WRITE);
    if (dataFile) {
        dataFile.print(str);
        dataFile.close();
        loggingFail = 0;
        disableErrorLed();
    } else {
        Serial.print(F("Error opening file"));
        Serial.println(fileName);
        loggingFail = 1;
    }
}


void printDayTotal(uint16_t * dayTotal, uint8_t startIdx, uint8_t line) {
    memset(loggingString, 0, MAX_STRING);
    if (dayTotal[startIdx+1] > 0) {
        snprintf(loggingString, MAX_STRING, "%d: %dW %d: %dW", startIdx, dayTotal[startIdx],
            startIdx+1, dayTotal[startIdx+1]);
    } else if (dayTotal[startIdx] > 0) {
        snprintf(loggingString, MAX_STRING, "%d: %dW", startIdx, dayTotal[startIdx]);
    }
    if (loggingString[0]) {
      displayWrite(&u8g, loggingString, line);
    }
}


uint8_t lastHourIdx;
#define LASTHOUR_MAX 60
uint8_t lastHour[LASTHOUR_MAX];
uint8_t lastDayIdx;
#define LASTDAY_MAX 24
uint8_t lastDay[LASTDAY_MAX];
#define DAYTOTAL_MAX 6
uint16_t dayTotal[DAYTOTAL_MAX];
uint16_t todayPeak;
uint16_t cumulativeNumber;


void loop() {
    if (triggerLogging) {
        cli();
        triggerLogging--;
        sei();

        // Read current voltage
        // Convert current reading to actual current. 0-5V = 0-53W
        // Using 53Watts max because at 5V given 0.4715 resistance, we will have 53 watts.
        // TODO use exact measured resistance
        currentReading = map(analogRead(A7), 0, 1023, 0, 530);
        if (((currentReading + 5) / 10) > todayPeak) {
            todayPeak = (currentReading + 5) / 10;
        }
        now = rtc.now();

        // Construct current time string and log it to the datalog file
        memset(loggingString, 0, MAX_STRING);
        snprintf(loggingString, MAX_STRING, "%d/%d/%d %d:%d:%d %d.%d\n", now.year(), now.month(),
            now.day(), now.hour(), now.minute(), now.second(), currentReading / 10, currentReading % 10);
        writeDataToFile("datalog.txt", loggingString);
        Serial.print(loggingString);

        lastHour[lastHourIdx++] = (uint8_t) ((currentReading + 5) / 10);
        if (now.minute() == 0 || lastHourIdx == LASTHOUR_MAX) {
            cumulativeNumber = 0;
            for (uint8_t i = 0; i < lastHourIdx; i++) {
                cumulativeNumber += lastHour[i];
            }
            lastDay[lastDayIdx++] = cumulativeNumber / (lastHourIdx + 1);
            memset(loggingString, 0, MAX_STRING);
            snprintf(loggingString, MAX_STRING, "%d/%d/%d %d:%d:%d Hour Total: %d\n", now.year(),
                now.month(), now.day(), now.hour(), now.minute(), now.second(), lastDay[lastDayIdx-1]);
            writeDataToFile("hourly.txt", loggingString);
            lastHourIdx = 0;
        }

        if ((now.hour() == 0 && now.minute() == 0) || lastDayIdx == 24) {
            todayPeak = 0;
            cumulativeNumber = 0;
            for (uint8_t i = 0; i < lastDayIdx; i++) {
              cumulativeNumber += lastDay[i];
            }
            for (uint8_t i = 0; i < LASTDAY_MAX; i++) {
              lastDay[i] = 0;
            }
            lastDayIdx = 0;
            memset(loggingString, 0, MAX_STRING);
            snprintf(loggingString, MAX_STRING, "%d/%d/%d %d:%d:%d Day Total: %d\n", now.year(),
                now.month(), now.day(), now.hour(), now.minute(), now.second(), cumulativeNumber);
            writeDataToFile("daily.txt", loggingString);
            for (int i = DAYTOTAL_MAX - 2; i >= 0; i--) {
                dayTotal[i+1] = dayTotal[i];
            }
            dayTotal[0] = cumulativeNumber;
        }

        u8g.firstPage();
        do {
            // Row 1: Last reading
            memset(loggingString, 0, MAX_STRING);
            snprintf(loggingString, MAX_STRING, "%d.%dW %dM %d Max", currentReading / 10,
                currentReading % 10, lastHourIdx, todayPeak);
            displayWrite(&u8g, loggingString, 1);
            // Row 2: Today's total
            if (lastDayIdx > 0 || dayTotal[0]) {
                cumulativeNumber = 0;
                for (uint8_t i = 0; i < LASTDAY_MAX; i++) {
                    cumulativeNumber += lastDay[i];
                }
                memset(loggingString, 0, MAX_STRING);
                snprintf(loggingString, MAX_STRING, "Today: %dW %dH", cumulativeNumber, lastDayIdx);
                displayWrite(&u8g, loggingString, 2);
            }

            // Row 3-6: KWH in previous days, total
            printDayTotal(dayTotal, 0, 3);
            printDayTotal(dayTotal, 2, 4);
            printDayTotal(dayTotal, 4, 5);
        } while (u8g.nextPage());
    }
}

