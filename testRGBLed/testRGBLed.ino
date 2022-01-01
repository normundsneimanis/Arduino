#include "RGBLed.h"
#define Green 6
#define Blue 9
#define Red 10
//#define Green 2
//#define Blue 3
//#define Red 6
#define testLed 13

RGBLed led(Red, Green, Blue);

void setup() {
  // put your setup code here, to run once:
  led.init();
  pinMode(testLed, OUTPUT);
  pinMode(Green, OUTPUT);
}

void loop() {
  led.blue();
  //analogWrite(testLed, 255);
  digitalWrite(testLed, HIGH);
  delay(500);
  digitalWrite(testLed, LOW);
  led.green();
  delay(500);
  led.red();
  delay(500);
  led.yellow();
  delay(500);
  led.white();
  delay(500);
}
