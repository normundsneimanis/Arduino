#include "TwoSevenSegLEDs.h"
#include "RotaryResistanceSensor.h"

// 7-segment LED control
#define clockResetPin 2
#define clockPin 4
#define clockPin2 12
#define potentiometerPin 15

TwoSevenSegLEDs segmentLeds(clockPin, clockPin2, clockResetPin);
RotaryResistanceSensor pot(potentiometerPin);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  segmentLeds.init();
  pot.init();
}

int i = 0;
int adder = 1;

void loop() {
  pot.readSensor();
  // put your main code here, to run repeatedly:
  segmentLeds.setLed(pot.getLedValue());
  Serial.print(F(" Led: "));
  Serial.println(pot.getLedValue());
  i +=  adder;
  if (i == 99) {
    adder = -1;
  } else if (i == 0) {
    adder = 1;
  }
  delay(50);

}
