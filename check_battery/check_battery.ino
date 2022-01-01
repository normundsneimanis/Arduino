/*
 * NOTE:
 * When changing resistor, change RESISTANCE define to actual resistanace
 * When changing cutoff voltage, change CUTOFF_VOLTAGE
 * 
 * Hardware wiring
 * 
 * temperature sensor currently disabled
 * Port2 -> KTY81-210 temperature sensor -> 2K Ohm resistor -> GND
 *                                       -> A6
 * 
 * Battery sense
 * A0..A3 - Battery +
 * GND - Battery -
 * 
 * Cutoff circuit
 * IRF3205 pin 0 - Arduino pin 3..7
 *         pin 1 - Battery after resistor
 *         pin 2 - Battery GND, Arduino GND
 * 
 * Display
 *    A5 - SCL
 *    A4 - SDA
 * 
 * Display switch button
 * GND -> button -> 9
 */


#include "timedelay.h"
/*
 * Resistance for ~500mA from 3.7V battery - 7.55 Ohm can be achieved with
 *  -- 6.8R -- 4.7R -- (11.5)
 *  --     22R     --
 *  
 */

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
 
#define RESISTANCE 8.3
#define CUTOFF_VOLTAGE 2.7

#include <Wire.h>  // Comes with Arduino IDE
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
#include <LiquidCrystal_I2C.h>

const int voltagePorts[4] = {A0, A1, A2, A3};
const int cutoffPorts[4] = {3, 4, 5, 6};

//#define voltageOutPort 2
//#define temperaturePort A6

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

/*
 * Temperature mapping
 * Starts with -55, ends with 150, step is 5 degrees
 */
//#define MAX_TEMPS 44
//const int temps[] = {
//  980, 1030, 1030 + 53, 1135, 1135 + 56, 1247, 1247 + 60, 1367, 1367 + 64,
//  1495, 1495 + 68, 1630, 1630 + 71, 1772, 1772 + 75, 1922, 2000, 2080, 2080 + 83,
//  2245, 2245 + 86, 2417, 2417 + 90, 2597, 2597 + 94, 2785, 2785 + 98, 2980,
//  2980 + 101, 3182, 3182 + 105, 3392, 3392 + 108, 3607, 3607+105, 3817, 3817+49,
//  3915, 3915 + 47, 4008, 4008 + 79, 4166, 4166 + 57, 4280
//};

timedelay readingDelay(1000L * 1); // 1 second
timedelay secondDelay(1000);

int voltageDivider(int Vin, int R1, int R2) {
  // calculate voltage divider and convert to Arduino input value
  // Vout = ((Vin * R2) / (R1 + R2))
//  Serial.print("Input: ");
//  Serial.print(R1);
//  Serial.print(" Divider: ");
//  Serial.print((((float) Vin * R2) / ((float) R1 + R2)));
//  Serial.print(" multiplier: ");
//  Serial.print((float) 5.0 / 1024.0, 6);
//  Serial.print(" result(float): ");
//  Serial.print(((float) ((Vin * R2) / (R1 + R2)) / ((float) 5.0 / 1024.0)), 2);
//  Serial.print(" Divider result: ");
//  Serial.println((int) ((float) (((float) Vin * R2) / (R1 + R2)) / ((float) 5.0 / 1024.0) + 0.5));
  return (int) ((float) (((float) Vin * R2) / (R1 + R2)) / ((float) 5.0 / 1024.0) + 0.5);
}

/*
 * Temperature sensor KTY81/210 used in voltage divider
 * +5v -- temperature sensor -- analog port -- 2kOhm resistor -- ground
 * TODO: Instead of using loops, use simple math to calculate the place
 */
//int toCelsius(int reading) {
//  // 1030 is 3.3V (676) -> -50 degrees celsius
//  // 4008 is 1.664V (340) > 130 degrees celsius
//  //return map(reading, 676, 340, -50, 130);
//
//  // Find closest reading from temperatures converted to voltage
//  int voltage1, voltage2;
//  int tempIdx = 0;
////  Serial.print("Reading is: ");
////  Serial.println(reading);
//  while (1) {
//    voltage1 = voltageDivider(5, temps[tempIdx], 2000);
//    voltage2 = voltageDivider(5, temps[tempIdx+1], 2000);
//    if (reading >= voltage2 && reading <= voltage1) {
//      break;
//    }
//    // handle error condition
//    if (tempIdx == MAX_TEMPS) {
//      return -160;
//    }
//    tempIdx++;
//  }
////  Serial.print("Index is: ");
////  Serial.println(tempIdx);
////
////  Serial.print("Range from: ");
////  Serial.print(voltage2);
////  Serial.print("Range to: ");
////  Serial.println(voltage1);
//  
//  // Starts @-55, each step is 5 degrees celsius;
//  int startTemp = -55;
//  for (int i = 0; i < tempIdx; i++) {
//    startTemp += 5;
//  }
//
////  Serial.print("Reading is: ");
////  Serial.println(reading);
////
////  Serial.print("Found start temperature: ");
////  Serial.println(startTemp);
//
//  // Where exactly in the 5 degree range are we?
//  int range = voltage1 - voltage2;
////  Serial.print(" One degree is: ");
//  range /= 5;
////  Serial.println(range);
//
//  // closest one is
//  voltage2 = voltage1;
//  voltage1 -= range;
////Serial.print("Starting with V1: ");
////Serial.print(voltage1);
////Serial.print(" V2: ");
////Serial.println(voltage2);
//  // TODO replace this loop with simple calculation
//  for (int i = 0; i < 5; i++) {
//    if (reading >= voltage1 && reading <= voltage2) {
//      startTemp++;
//      break;
//    }
//    startTemp++;
//    voltage1 -= range;
//    voltage2 -= range;
////Serial.print("V1: ");
////Serial.print(voltage1);
////Serial.print(" V2: ");
////Serial.println(voltage2);
//  }
//
//  return startTemp;
//}

// Testing temperature sensor readings
//  Serial.print("980(-55) is ");
//  Serial.println(toCelsius(voltageDivider(5, 980, 2000)));
//  Serial.print("1030(-50) is ");
//  Serial.println(toCelsius(voltageDivider(5, 1030, 2000)));
//  Serial.print("1040(-49) is ");
//  Serial.println(toCelsius(voltageDivider(5, 1040, 2000)));
//  Serial.print("1070(-46) is ");
//  Serial.println(toCelsius(voltageDivider(5, 1070, 2000)));
//  Serial.print("4008(130) is ");
//  Serial.println(toCelsius(voltageDivider(5, 4008, 2000)));
//  
//  Serial.print("1630(0) is ");
//  Serial.println(toCelsius(voltageDivider(5, 1630, 2000)));
//  Serial.print("1772(10) is ");
//  Serial.println(toCelsius(voltageDivider(5, 1772, 2000)));
//  Serial.print("1922(20) is ");
//  Serial.println(toCelsius(voltageDivider(5, 1922, 2000)));
//  Serial.print("2000(25) is ");
//  Serial.println(toCelsius(voltageDivider(5, 2000, 2000)));
//  Serial.print("2080(30) is ");
//  Serial.println(toCelsius(voltageDivider(5, 2080, 2000)));
//  Serial.print("2417(50) is ");
//  Serial.println(toCelsius(voltageDivider(5, 2417, 2000)));


void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) {
    initBattery(&battery[i]);
    pinMode(voltagePorts[i], INPUT);
    pinMode(cutoffPorts[i], OUTPUT);
    digitalWrite(cutoffPorts[i], HIGH);
  }
//  pinMode(voltageOutPort, OUTPUT);
//  pinMode(temperaturePort, INPUT);
  lcd.begin(20,4);   // initialize the lcd for 20 chars 4 lines, turn on backlight
}


//int temperatureReading;
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
//    digitalWrite(voltageOutPort, HIGH);
//    temperatureReading = analogRead(temperaturePort);
//    digitalWrite(voltageOutPort, LOW);
//    // Convert temperature reading to degrees celsius
//    temperatureReading = toCelsius(temperatureReading) - 5;
//    lcd.setCursor(0,1);
//    lcd.print("Temperature: ");
//    lcd.setCursor(13,1);
//    lcd.print(temperatureReading);

    // First, make the readings for each battery and calculate voltage
    for (int i = 0; i < 4; i++) {
      battery[i].voltageReading = analogRead(voltagePorts[i]);
      battery[i].voltage = (float) ((float) map(battery[i].voltageReading, 0, 1023, 0, 500) / 100.0);
    }

    // Calculate voltage end decide if we need to cutoff this battery
    for (int i = 0; i < 4; i++) {
      if (battery[i].droppedBelow == 1) {
        continue;
      }
      // Calculate ampere hours in battery
      battery[i].totalAh += ((battery[i].voltage / RESISTANCE) / 60.0 / 60.0);
      if (map(battery[i].voltageReading, 0, 1023, 0, 500) < CUTOFF_VOLTAGE * 100) {
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

//    // after cutoff display
//        lcd.setCursor(0,4);
//        lcd.print("< ");
//        lcd.print(CUTOFF_VOLTAGE);
//        lcd.print("V @ ");
//        lcd.setCursor(10,4);
//        lcd.print(hours);
//        lcd.setCursor(12,4);
//        lcd.print(":");
//        lcd.setCursor(13,4);
//        lcd.print(minutes);
//
//    lcd.setCursor(20-5,0);
//    lcd.print(voltage, 2);
//    lcd.print("V");
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


    // Print battery data
    /*
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
//    if (!droppedBelow) {
//      lcd.setCursor(5,2);
//      lcd.print(totalAh * 1000.0, 1);
//      lcd.setCursor(0,2);
//      lcd.print("mAh:");
//    }
  }
}
