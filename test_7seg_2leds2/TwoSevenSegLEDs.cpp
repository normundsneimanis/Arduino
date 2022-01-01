#include "Arduino.h"
#include "TwoSevenSegLEDs.h"

TwoSevenSegLEDs::TwoSevenSegLEDs(int pin1, int pin2, int resetPin) {
  // 7-segment led config
  _clockPin = pin1;
  _clockPin2 = pin2;
  _clockResetPin = resetPin;
  _previousNumber = -1;
}


/*
 * Sends a clock pulse to the counter making it advance.
 */
inline void TwoSevenSegLEDs::clock(int pin) {
  digitalWrite(pin,HIGH);
  digitalWrite(pin,LOW);
}

void TwoSevenSegLEDs::init() {
	pinMode(_clockPin, OUTPUT);
	pinMode(_clockPin2, OUTPUT);
	pinMode(_clockResetPin, OUTPUT);
}

void TwoSevenSegLEDs::resetLed(void) {
  digitalWrite(_clockResetPin,HIGH);
  digitalWrite(_clockResetPin,LOW);
}

void TwoSevenSegLEDs::setLed(int number) {
  if (number > 99) {
    number = 99;
  }
  if (number == _previousNumber) {
    return;
  }
  _previousNumber = number;
  int secondDigit = (number % 10);
  int firstDigit = (number / 10);
  this->resetLed();
  while(secondDigit > 0 || firstDigit > 0) {
    if (secondDigit-- > 0) {
      this->clock(_clockPin2);
    }
    if (firstDigit-- > 0) {
      this->clock(_clockPin);
    }
  }
}
