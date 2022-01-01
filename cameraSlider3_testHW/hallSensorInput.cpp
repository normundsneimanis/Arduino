#include "Arduino.h"
#include "hallSensorInput.h"

hallSensorInput::hallSensorInput(int leftPin, int rightPin) {
	_rightPin = rightPin;
	_leftPin = leftPin;
}

void hallSensorInput::init() {
	pinMode(_rightPin, INPUT);
	pinMode(_leftPin, INPUT);
}

void hallSensorInput::readSensors() {
	_leftValue = analogRead(_leftPin);
	_rightValue = analogRead(_rightPin);
}

int hallSensorInput::getRawLeft() {
  return _leftValue;
}

int hallSensorInput::getRawRight() {
  return _rightValue;
}

int hallSensorInput::endReached() {
	if (_leftValue < 500) {
		return 1;
	} else if (_rightValue < 500) {
		return 1;
	}
	return 0;
}

int hallSensorInput::rightReached() {
	return (_rightValue < 500);
}

int hallSensorInput::leftReached() {
	return (_leftValue < 500);
}

