#include <EEPROM.h>
#include "RGBLed.h"
#include "TimeLapse.h"
#include "timedelay.h"
#include "DCMotor.h"
#include "RotaryResistanceSensor.h"
#include "TwoSevenSegLEDs.h"

// Arduino Uno R3 Pinout
//static int clockPin = 13;
//static int clockPin2 = 12;
//static int clockResetPin = 11;
//static int motorPin1 = 9;
//static int motorPin2 = 6;
// Analog pins
//static int sensorPin = 0;
//static int hallSensor1 = 1;
//static int hallSensor2 = 2;

// Arduino Pro mini pinout
// 7-segment LED control
// Control 7-segment LED 1st digit
#define clockPin 3
// Control 7-segment LED 2nd digit
#define clockPin2 4    
// Reset both 7-segment LED displays to zero
#define clockResetPin 5
// Motor direction control
#define motorPin1 10
#define motorPin2 11
// Analog input pins
// 14 - A0. Speed setting slide resistor
#define sensorPin 14
// 19 - A6
#define hallSensor1 19
// 22 - A7
#define hallSensor2 22
// Digital input pins
// (button) Pin to detect if movement should be repeated
#define repeatPin 6
// if ON, do time lapse, very slow movement
#define timeLapsePin 7
// Input on which direction to go
#define motorDirectionPin1 8
// Input on which direction to go
#define motorDirectionPin2 9
// Time Lapse configuration buttons - digital.
// 15 - A1
#define configuringTLDrivingTimePin 15
// 16 - A2
#define configuringTLWaitTimePin 16
// TimeLapse configuration status RGB LED connections
// 17 - A3
#define TLRedPin 17
// 18 - A4
#define TLGreenPin 18
// 19 - A5
#define TLBluePin 19

int stopFlag = 0; // Flags that motor needs to be stopped. 0 - continue; 1 - stopped

int clockCounter = 0;
#define sleeptime 100
#define timeLapseInterval 30000
int doTimeLapse = -1;
int sensorValue;
int sensorValuePrev;
int motorValue;
int hallSensor1Value;
int hallSensor2Value;
DCMotorDirection motorDirection = STOP; // 0 - stop; 1 - right; 2 - left
DCMotorDirection motorDirectionPrev;
// Does slider repeats sliding or stops when reaches the end
int repeatedSlide = 0; // 0 - stop; 1 - repeat
int ledValue;
int motorDirection1;
int motorDirection2;
int repeatSlideInProgress = 0;
int configuringTLDrivingTime;
int configuringTLWaitTime;

timedelay StatisticsDelay(1000); // Will print statistics 1000 - every second
TwoSevenSegLEDs segmentLeds(clockPin, clockPin2, clockResetPin);
DCMotor motor(motorPin1, motorPin2);
RotaryResistanceSensor pot(sensorPin);


void setup() {
  Serial.begin(9600);
  // Hall effect sensors
  pinMode(hallSensor1, INPUT);
  pinMode(hallSensor2, INPUT);
  // Repeat button
  pinMode(repeatPin, INPUT);
  // Time Lapse enable
  pinMode(timeLapsePin, INPUT);
  // Motor control switches
  pinMode(motorDirectionPin1, INPUT);
  pinMode(motorDirectionPin2, INPUT);
  // Time Lapse configuration buttons
  pinMode(configuringTLDrivingTimePin, INPUT);
  pinMode(configuringTLWaitTimePin, INPUT);
}


void loop() {
  StatisticsDelay.cycle();
  // Read all input sensors/buttons
  repeatedSlide = digitalRead(repeatPin);
  sensorValue = pot.readSensor();
  motorDirection1 = digitalRead(motorDirectionPin1);
  motorDirection2 = digitalRead(motorDirectionPin2);
  hallSensor1Value = analogRead(hallSensor1);
  hallSensor2Value = analogRead(hallSensor2);
  doTimeLapse = digitalRead(timeLapsePin);
  configuringTLDrivingTime = digitalRead(configuringTLDrivingTimePin);
  configuringTLWaitTime = digitalRead(configuringTLWaitTimePin);

  // Calculate real requested direction. This will be used 
  // both in normal and timelapse operation
  if ((motorDirection1) && (!motorDirection2)) {
    if (!repeatSlideInProgress) {
      stopFlag = 0;
      motorDirection = RIGHT;
    }
  } else if ((!motorDirection1) && (motorDirection2)) {
    if (!repeatSlideInProgress) {
      stopFlag = 0;
      motorDirection = LEFT;
    }
  } else {
    repeatSlideInProgress = 0;
    stopFlag = 1;
  }

  // Detect that movement has reached the end
  if ((hallSensor1Value < 500) && (hallSensor2Value < 500)) {
    motorDirection = STOP;
    stopFlag = 1;
  } else if (hallSensor1Value < 500) {
    motorDirectionPrev = motorDirection;
    motorDirection = RIGHT;
    if (!repeatedSlide) {
      if (!motorDirection1) {
        stopFlag = 1;
      }
    } else {
      if (motorDirectionPrev != motorDirection) {
        reverseNow = 1;
      }
      repeatSlideInProgress = 1;
      stopFlag = 0;
    }
  } else if (hallSensor2Value < 500) {
    motorDirectionPrev = motorDirection;
    motorDirection = LEFT;
    if (!repeatedSlide) {
      if (!motorDirection2) {
        stopFlag = 1;
      }
    } else {
      if (motorDirectionPrev != motorDirection) {
        reverseNow = 1;
      }
      repeatSlideInProgress = 1;
      stopFlag = 0; 
    }
  }
  
  if (doTimeLapse) {
    // All time lapse control functionality goes here
    // Enter Wait time configuration mode, if requested.
    while (configuringTLDrivingTime) {
      segmentLeds.setLed("01");
      // Save configuration, if changed
      configuringTLDrivingTime = digitalRead(configuringTLDrivingTimePin);
    }
    while (configuringTLWaitTime) {
      segmentLeds.setLed("02");
      // Read settings while button is pressed
      // Save configuration, if changed
      configuringTLWaitTime = digitalRead(configuringTLWaitTimePin);
    }

    // If motor movement time is less than or equal to 200mSec, move it here in while loop
    // Otherwise continue operation and check when motor should be started or stopped
    // After motor is stopped due to either two of previous reasons, reset timers, sleep 50 msec and take a picture
    
    if (motorDirection == STOP || stopFlag == 1) {
      motor.stop();
    } else if (reverseNow) {
      motor.setDirection(motorDirection);
      motor.setSpeed(motorValue);
      motor.commit();
      reverseNow = 0;
    } else if (motorDirection == RIGHT || motorDirection == LEFT) {
      motor.setDirection(motorDirection);
      motor.setSpeed(motorValue);
      motor.commit();
    }
  } else {
    // Normal camera slider functionality follows
    if (StatisticsDelay.hasPassed()) {
      Serial.print("RawPot: ");
      Serial.print(sensorValue);
      Serial.print(" Rep: ");
      Serial.print(repeatedSlide);
      Serial.print(" RepP: ");
      Serial.print(repeatSlideInProgress);
      Serial.print(" Stop: ");
      Serial.print(stopFlag);
      Serial.print(" MD1: ");
      Serial.print(motorDirection1);
      Serial.print(" MD2: ");
      Serial.print(motorDirection2);
    }
    
    ledValue = pot.ledValue();
    motorValue = pot.normalizeValue() * 2.5758;
  
    // Set motor speed according to required speed, direction and stopFlag
    if (motorDirection == STOP || stopFlag == 1) {
      motor.smoothStop();
    } else if (reverseNow) {
      motor.reverse();
      reverseNow = 0;
    } else if (motorDirection == RIGHT || motorDirection == LEFT) {
      motor.setDirection(motorDirection);
      motor.setSpeed(motorValue);
      motor.commit();
    } else {
      while (1) {
        Serial.print("Error detecting motor direction. It is ");
        Serial.println(motorDirection);
        segmentLeds.setLed(0);
        delay(500);
        segmentLeds.setLed(98);
        delay(500);
      }
    }
    segmentLeds.setLed(ledValue);
    if (StatisticsDelay.hasPassed()) {
      StatisticsDelay.reset();
      Serial.print(" Pot: ");
      Serial.print(sensorValue);
      Serial.print(" PWM: ");
      Serial.print(motorValue);
      Serial.print(" H1: ");
      Serial.print(hallSensor1Value);
      Serial.print(" H2: ");
      Serial.print(hallSensor2Value);
      Serial.print(" Dir: ");
      Serial.println(motorDirection);
    }
    delay(sleeptime);
  }
}
