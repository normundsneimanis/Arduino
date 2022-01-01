#include "Arduino.h"
#include "userInput.h"

userInput::userInput(int motorDirectionLeftPin, int motorDirectionRightPin) {
	_motorDirectionLeftPin = motorDirectionLeftPin;
	_motorDirectionRightPin = motorDirectionRightPin;
}

void userInput::init() {
	pinMode(_motorDirectionLeftPin, INPUT);
	pinMode(_motorDirectionRightPin, INPUT);
	digitalWrite(_motorDirectionLeftPin, HIGH);
	digitalWrite(_motorDirectionRightPin, HIGH);
}

void userInput::readSensors() {
	_motorDirectionLeftReading = digitalRead(_motorDirectionLeftPin);
	_motorDirectionRightReading = digitalRead(_motorDirectionRightPin);
}

// Calculate real requested direction. This is used
// both in normal and timelapse operation
userInputDirection userInput::getDirection() {
	userInputDirection direction;
	if ((_motorDirectionLeftReading) && (!_motorDirectionRightReading)) {
	  direction = UI_LEFT;
	} else if ((!_motorDirectionLeftReading) && (_motorDirectionRightReading)) {
	  direction = UI_RIGHT;
	} else {
		direction = UI_STOP;
	}
	
	return direction;
}
