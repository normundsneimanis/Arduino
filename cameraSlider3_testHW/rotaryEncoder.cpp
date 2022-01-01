#include "Arduino.h"
#include "rotaryEncoder.h"

rotaryEncoder::rotaryEncoder(int pinLeft, int pinRight, int buttonPin) {
	_pinLeft = pinLeft;
	_pinRight = pinRight;
	_buttonPin = buttonPin;
	_currentPosition = 0;
	_step = 1;
	_min = -100;
	_max = 100;
	_buttonState = 0;
  _pushTime = 1000;
  _pusStartTime = ULONG_MAX - 10000;
}

void rotaryEncoder::init() {
	pinMode(_pinLeft, INPUT); 
	digitalWrite(_pinLeft, HIGH);       // turn on pullup resistor
	pinMode(_pinRight, INPUT); 
	digitalWrite(_pinRight, HIGH);       // turn on pullup resistor
	pinMode(_buttonPin, INPUT);
	digitalWrite(_buttonPin, HIGH);       // turn on pullup resistor
}

void rotaryEncoder::setPushTime(int tm) {
  _pushTime = tm;
}

int rotaryEncoder::buttonLongPressed() {
  if (millis() > _pusStartTime + _pushTime) {
    _pusStartTime = ULONG_MAX - 10000;
    _buttonState = 0;
    return 1;
  } else {
    return 0;
  }
}

void rotaryEncoder::stepUp() {
	_currentPosition += _step;
	if (_currentPosition > _max) {
		_currentPosition = _max;
	}
}

void rotaryEncoder::stepDown() {
	_currentPosition -= _step;
	if (_currentPosition < _min) {
		_currentPosition = _min;
	}
}

void rotaryEncoder::setStep(int step) {
	_step = step;
}

void rotaryEncoder::setCurrentPosition(int position) {
	_currentPosition = position;
}

int rotaryEncoder::getCurrentPosition() {
	return _currentPosition;
}

int rotaryEncoder::buttonPressed() {
	if (_buttonState == 0 && (!digitalRead(_buttonPin))) {
		_buttonState = 1;
    _pusStartTime = millis();
		return 1;
	} else if ((_buttonState == 1) && (digitalRead(_buttonPin))) {
		_buttonState = 0;
    _pusStartTime = ULONG_MAX - 10000;
		return 0;
	}
	//Serial.println("Fail in rotaryEncoder::buttonPressed()");
	return 0;
}

int rotaryEncoder::buttonPressedReleased() {
  if (_buttonState == 0 && (!digitalRead(_buttonPin))) {
    _buttonState = 1;
    _pusStartTime = millis();
    return 0;
  } else if ((_buttonState == 1) && (digitalRead(_buttonPin))) {
    _buttonState = 0;
    _pusStartTime = ULONG_MAX - 10000;
    return 1;
  }
  //Serial.println("Fail in rotaryEncoder::buttonPressed()");
  return 0;
}

void rotaryEncoder::setLimits(int min, int max) {
	_min = min;
	_max = max;
}

