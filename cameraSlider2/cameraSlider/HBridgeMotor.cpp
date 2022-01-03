#include "HBridgeMotor.h"
#ifndef TEST
#include "Arduino.h"
#else
#include "mockArduino.h"
SerialClass Serial;
extern int TCCR1B;
#endif

HBridgeMotor::HBridgeMotor(int leftPin, int rightPin) {
	_pins[HBM_RIGHT] = rightPin;
	_pins[HBM_LEFT] = leftPin;
	_direction = HBM_STOP;
}

void HBridgeMotor::init() {
	_motorLeft = new DCMotor(_pins[HBM_LEFT]);
	_motorRight = new DCMotor(_pins[HBM_RIGHT]);
	_motorLeft->init();
	_motorRight->init();
	this->smoothEnable(1);
	this->_setMotorPWMFreq();
}

int HBridgeMotor::isRunning() {
	return (_direction != HBM_STOP);
}

void HBridgeMotor::setDirection(HBridgeMotorDirection mdirection) {
	if (mdirection == HBM_LEFT || mdirection == HBM_RIGHT || mdirection == HBM_STOP) {
    //Serial.print(F(" Setting direction: "));
    //Serial.println(mdirection);
		_direction = mdirection;
	}
}

void HBridgeMotor::stop() {
	_direction = HBM_STOP;
	this->commit();
}

void HBridgeMotor::start() {
	this->commit();
}

void HBridgeMotor::smoothEnable(byte enable) {
	if (enable) {
		_smoothEnable = 1;
	} else {
		_smoothEnable = 0;
	}
	_motorLeft->setSmooth(_smoothEnable);
	_motorRight->setSmooth(_smoothEnable);
}

HBridgeMotorDirection HBridgeMotor::getDirection() {
	return _direction;
}

void HBridgeMotor::commit() {
	if (_direction == HBM_STOP) {
		_motorLeft->setSpeed(0);
		_motorLeft->commit();
		_motorRight->setSpeed(0);
		_motorRight->commit();
	} else if (_direction == HBM_LEFT) {
    //Serial.println(F(" Going Left."));
		_motorRight->setSpeed(0);
		_motorRight->commit();
		_motorLeft->setSpeed(_speed);
		_motorLeft->commit();
	} else if (_direction == HBM_RIGHT) {
     //Serial.println(F(" Going Right."));
		_motorLeft->setSpeed(0);
		_motorLeft->commit();
		_motorRight->setSpeed(_speed);
		_motorRight->commit();
	}
}

void HBridgeMotor::setSpeed(int speed) {
	if (speed > 255) {
		speed = 255;
	} else if (speed < 0) {
		speed = 0;
	}
	_speed = speed;
}

int HBridgeMotor::getSpeed() {
  return _speed;
}


/* 
 * Reverses direction of a running motor. If motor is stopped, 
 * only direction is reversed, but motor is left stopped.
 */
void HBridgeMotor::reverse() {
	if (_direction == HBM_RIGHT) {
		_direction = HBM_LEFT;
	} else if (_direction == HBM_LEFT) {
		_direction = HBM_RIGHT;
	} else {
		return;
	}
	this->commit();
}

// Sets frequency good for most silent motor control
inline void HBridgeMotor::_setMotorPWMFreq() {
	TCCR1B = (TCCR1B & (0b11111000 | 0b00000100));    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
	//TCCR1B = (TCCR1B & (B11111000 | B00000101));    // set timer 1 divisor to  1024 for PWM frequency of    30.64 Hz
	//TCCR1B = (TCCR1B & (B11111000 | B00000001));    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
	//TCCR1B = (TCCR1B & (B11111000 | B00000010));    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
}
