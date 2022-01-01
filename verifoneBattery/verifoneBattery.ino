/*
 * vim: tabstop=4 shiftwidth=4 expandtab syntax=c
 *
 * NOTE:
 * When changing resistor, change RESISTANCE define to actual resistanace
 * When changing cutoff voltage, change CUTOFF_VOLTAGE
 * 
 * Hardware wiring
 * 
 * Battery sense
 * Battery + -> 1K resistor -> A0..A3 -> 1K resistor -> GND
 * GND - Battery -
 * 
 * Cutoff circuit (Mosfet pins from the left side)
 * IRF3205 pin 1 - Gate
 *         pin 2 - Drain
 *         pin 3 - Source
 *         pin 4 - Drain
 * IRF3205 pin 1 - Arduino pin 3..7
 *         pin 2 - Battery after resistor
 *         pin 3 - Battery GND, Arduino GND
 * 
 * Display
 *    A5 - SCL
 *    A4 - SDA
 * 
 */


#include "timedelay.h"
/*
 * Resistance for ~1A from 7.2V battery - 8.2 Ohm can be achieved with
 * standard resistor
 *
 * For uC to properly read voltage, let's use same value resistor voltage
 * divider where 10V is converted to 5V
 *
 * Cutoff voltage would be 2.7V*2 = 5.4V
 *
 */
// TODO measure exact resistance of power resistors
#define RESISTANCE 17.95
#define CUTOFF_VOLTAGE 2.7
#define INPUT_VOLTAGE 503

typedef struct {
  float voltage;
  double totalAh;
  int voltageReading;
  int droppedBelow;
  int hours; // Cutoff hour
  int minutes; // Cutoff minute
 } battery_t;

battery_t battery[4];

void initBattery(battery_t *batt) {
  batt->voltageReading = 0;
  batt->voltage = 0.0;
  batt->totalAh = 0.0;
  batt->droppedBelow = 0;
  batt->hours = 0;
  batt->minutes = 0;
}
 

#include <Wire.h>  // Comes with Arduino IDE
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
#include <LiquidCrystal_I2C.h>

const int voltagePorts[4] = {A0, A1, A2, A3};
const int cutoffPorts[4] = {3, 4, 5, 6};

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address


timedelay readingDelay(1000L * 1); // 1 second
timedelay secondDelay(1000L);


void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) {
    initBattery(&battery[i]);
    pinMode(voltagePorts[i], INPUT);
    pinMode(cutoffPorts[i], OUTPUT);
    digitalWrite(cutoffPorts[i], HIGH);
  }
  lcd.begin(20,4); // initialize the lcd for 20 chars 4 lines, turn on backlight
}


int seconds = 0;
int minutes = 0;
int hours = 0;
bool blinker = false;

void loop() {
  secondDelay.cycle();
  readingDelay.cycle();

  // Read battery reading and calculate voltage, cutoff battery if necessary
  if (readingDelay.hasPassed()) {
    readingDelay.reset();
    // First, make the readings for each battery and calculate voltage
    for (int i = 0; i < 4; i++) {
      battery[i].voltageReading = analogRead(voltagePorts[i]);
      battery[i].voltage = (float) ((float) map(battery[i].voltageReading, 0, 1023, 0, 1000) / 100.0);
    }

    // Calculate voltage end decide if we need to cutoff this battery
    for (int i = 0; i < 4; i++) {
      if (battery[i].droppedBelow == 1) {
        continue;
      }
      // Calculate ampere hours in battery
      battery[i].totalAh += ((battery[i].voltage / RESISTANCE) / 60.0 / 60.0);
      if (map(battery[i].voltageReading, 0, 1023, 0, INPUT_VOLTAGE) < CUTOFF_VOLTAGE * 100) {
        battery[i].droppedBelow = 1;
        // disable the battery
        digitalWrite(cutoffPorts[i], LOW);
        // remember time, how long this battery lasted
        battery[i].hours = hours;
        battery[i].minutes = minutes;
        if (seconds > 30) {
          battery[i].minutes = minutes + 1;
        }
      }
    }
  }

  // print time on the display
  if (secondDelay.hasPassed()) {
    blinker = !blinker;
    secondDelay.reset();
    seconds++;
    if (seconds == 60) {
      minutes++;
      seconds = 0;
    }
    if (minutes == 60) {
      hours++;
      minutes = 0;
    }
    lcd.setCursor(0,0);
    lcd.print("  ");
    lcd.setCursor(0,0);
    lcd.print(hours);
    lcd.setCursor(2,0);
    if (blinker) {
      lcd.print(":");
    } else {
      lcd.print(" ");
    }
    lcd.setCursor(3,0);
    lcd.print("  ");
    lcd.setCursor(3,0);
    lcd.print(minutes);
    lcd.setCursor(5,0);
    if (blinker) {
      lcd.print(":");
    } else {
      lcd.print(" ");
    }
    lcd.setCursor(6,0);
    lcd.print("  ");
    lcd.setCursor(6,0);
    lcd.print(seconds);


    /*
     * Print battery data
     *
     * Battery 1 - second row 0-9 stop: 4th row, 0-4
     * Battery 2 - second row 10-19 stop: 4th row, 5-9
     * Battery 3 - third row 0-9 stop: 4th row, 10-14
     * Battery 4 - third row 10-19 stop: 4th row, 15-19
     */

    lcd.setCursor(0,1);
    lcd.print(battery[0].voltage, 1);
    lcd.print("V");
    lcd.print(battery[0].totalAh * 1000, 1);

    lcd.setCursor(10,1);
    lcd.print(battery[1].voltage, 1);
    lcd.print("V");
    lcd.print(battery[1].totalAh * 1000, 1);

    lcd.setCursor(0,2);
    lcd.print(battery[2].voltage, 1);
    lcd.print("V");
    lcd.print(battery[2].totalAh * 1000, 1);

    lcd.setCursor(10,2);
    lcd.print(battery[3].voltage, 1);
    lcd.print("V");
    lcd.print(battery[3].totalAh * 1000, 1);

    // print stop status
    lcd.setCursor(0,3);
    if (battery[0].droppedBelow) {
        lcd.print(battery[0].hours);
        lcd.print(":");
        lcd.print(battery[0].minutes);
    }
    lcd.setCursor(5,3);
    if (battery[1].droppedBelow) {
        lcd.print(battery[1].hours);
        lcd.print(":");
        lcd.print(battery[1].minutes);
    }
    lcd.setCursor(10,3);
    if (battery[2].droppedBelow) {
        lcd.print(battery[2].hours);
        lcd.print(":");
        lcd.print(battery[2].minutes);
    }
    lcd.setCursor(15,3);
    if (battery[3].droppedBelow) {
        lcd.print(battery[3].hours);
        lcd.print(":");
        lcd.print(battery[3].minutes);
    }
  }
}
