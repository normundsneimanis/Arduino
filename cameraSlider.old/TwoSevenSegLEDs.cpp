#include "Arduino.h"
#include "TwoSevenSegLEDs.h"

TwoSevenSegLEDs::TwoSevenSegLEDs(int pin1, int pin2, int resetPin) {
  // 7-segment led config
  clockPin = pin1;
  clockPin2 = pin2;
  clockResetPin = resetPin;
  pinMode(clockPin, OUTPUT);
  pinMode(clockPin2, OUTPUT);
  pinMode(clockResetPin, OUTPUT);
}


/*
 * Sends a clock pulse to the counter making it advance.
 */
inline void TwoSevenSegLEDs::clock(int pin) {
  digitalWrite(pin,HIGH);
  digitalWrite(pin,LOW);
}

void TwoSevenSegLEDs::resetLed(void) {
  digitalWrite(clockResetPin,HIGH);
  digitalWrite(clockResetPin,LOW);
}

void TwoSevenSegLEDs::setLed(int number) {
  static int previousNumber = -1;
  if (number > 99) {
    number = 99;
  }
  if (number == previousNumber) {
    return;
  }
  previousNumber = number;
  int secondDigit = (number % 10);
  int firstDigit = (number / 10);
  resetLed();
  while(secondDigit > 0 || firstDigit > 0) {
    if (secondDigit-- > 0) {
      this->clock(clockPin2);
    }
    if (firstDigit-- > 0) {
      this->clock(clockPin);
    }
  }
}
