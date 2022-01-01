#include "Arduino.h"
#include "RGBLed.h"

RGBLed::RGBLed(int redPin, int greenPin, int bluePin) {
	_redPin = redPin;
	_greenPin = greenPin;
	_bluePin = bluePin;
}

void RGBLed::init() {
	pinMode(_redPin, OUTPUT);
	pinMode(_greenPin, OUTPUT);
	pinMode(_bluePin, OUTPUT);
}

void RGBLed::green() {
  analogWrite(_redPin, 0);
  analogWrite(_greenPin, RGBLED_PWM);
  analogWrite(_bluePin, 0);
}

void RGBLed::blue() {
  analogWrite(_redPin, 0);
  analogWrite(_greenPin, 0);
  analogWrite(_bluePin, RGBLED_PWM);
}

void RGBLed::red() {
  analogWrite(_redPin, RGBLED_PWM);
  analogWrite(_greenPin, 0);
  analogWrite(_bluePin, 0);
}

void RGBLed::yellow() {
  analogWrite(_redPin, RGBLED_PWM);
  analogWrite(_greenPin, RGBLED_PWM);
  analogWrite(_bluePin, 0);
}

void RGBLed::white() {
  analogWrite(_redPin, RGBLED_PWM);
  analogWrite(_greenPin, RGBLED_PWM);
  analogWrite(_bluePin, RGBLED_PWM);
}

void RGBLed::off() {
  analogWrite(_redPin, 0);
  analogWrite(_greenPin, 0);
  analogWrite(_bluePin, 0);
}
