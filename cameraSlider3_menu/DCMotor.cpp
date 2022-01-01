#include "DCMotor.h"
#ifndef TEST
#include "Arduino.h"
#else
#include "mockArduino.h"
#endif


DCMotor::DCMotor(int pin) {
	_pin = pin;
	_currentSpeed = _newSpeed = 0;
	_smooth = 1;
}

void DCMotor::init() {
	pinMode(_pin, OUTPUT);
}

void DCMotor::commit() {
	if (_currentSpeed != _newSpeed) {
		this->adjustSpeed();
	}
}

void DCMotor::setSpeed(int speed) {
	if (speed > 255) {
		speed = 255;
	} else if (speed < 0) {
		speed = 0;
	}
	_newSpeed = speed;
}

void DCMotor::start() {
	_currentSpeed = _newSpeed;
	analogWrite(_pin, _currentSpeed);
}

void DCMotor::setSmooth(byte smooth) {
	if (smooth)
		_smooth = 1;
	else
		_smooth = 0;
}

void DCMotor::stop() {
	analogWrite(_pin, 0);
	_currentSpeed = 0;
}

void DCMotor::smoothStop() {
	this->_smoothStop(0);
}

void DCMotor::_smoothStop(int speed) {
	int counter = DCMotorDelay;
	int pwm = _currentSpeed;
	while(pwm > speed) {
		--pwm;
		analogWrite(_pin, pwm);
		if (counter > 0){
			counter--;
		} else {
			delay(1);
			counter = DCMotorDelay;
		}
	}
	_currentSpeed = speed;
}

void DCMotor::adjustSpeed() {
	if (_currentSpeed == _newSpeed) {
		return;
	} else if (_currentSpeed < _newSpeed) {
		if (_smooth)
			this->smoothStart();
		else
			this->start();
	} else {
		if (_smooth)
			this->_smoothStop(_newSpeed);
		else
			this->stop();

		_currentSpeed = _newSpeed;

	}
}

void DCMotor::smoothStart() {
	volatile int curpwm = _currentSpeed;
	volatile int counter = DCMotorDelay;
	int pwm = _newSpeed;
	while(curpwm < pwm) {
		++curpwm;
		analogWrite(_pin, curpwm);
		if (counter > 0){
			counter--;
		} else {
			delay(1);
			counter = DCMotorDelay;
		}
	}
	_currentSpeed = _newSpeed;
}
