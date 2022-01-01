#include "Arduino.h"
#include "RGBLed.h"

RGBLed::RGBLed(int redPin, int greenPin, int bluePin) {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  _redPin = redPin;
  _greenPin = greenPin;
  _bluePin = bluePin;
}

void RGBLed::green() {
  this->setLedPWMFreq();
  analogWrite(_redPin, 0);
  analogWrite(_greenPin, 111);
  analogWrite(_bluePin, 0);
}

void RGBLed::blue() {
  this->setLedPWMFreq();
  analogWrite(_redPin, 0);
  analogWrite(_greenPin, 111);
  analogWrite(_bluePin, 0);
}

void RGBLed::red() {
  this->setLedPWMFreq();
  analogWrite(_redPin, 111);
  analogWrite(_greenPin, 0);
  analogWrite(_bluePin, 0);
}

void RGBLed::yellow() {
  this->setLedPWMFreq();
  analogWrite(_redPin, 111);
  analogWrite(_greenPin, 111);
  analogWrite(_bluePin, 0);
}

void RGBLed::off() {
  analogWrite(_redPin, 0);
  analogWrite(_greenPin, 0);
  analogWrite(_bluePin, 0);
}

// Sets clock frequency good for driving 3.3V LED with 5V port
inline void RGBLed::setLedPWMFreq() {
  //TCCR1B = (TCCR1B & (0b11111000 | 0b00000100));    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR1B = (TCCR1B & (0b11111000 | 0b00000101));    // set timer 1 divisor to  1024 for PWM frequency of    30.64 Hz
  //TCCR1B = (TCCR1B & (0b11111000 | 0b00000001));    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
  TCCR1B = (TCCR1B & (0b11111000 | 0b00000010));    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
}
