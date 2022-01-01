#include "DCMotor.h"
#ifndef TEST
#include "Arduino.h"
#else
#include "mockArduino.h"
extern int TCCR1B;
#endif


DCMotor::DCMotor(int rightPin, int leftPin) {
   pinMode(rightPin, OUTPUT);
   pinMode(leftPin, OUTPUT);
   pins[RIGHT] = rightPin;
   pins[LEFT] = leftPin;
   _currentDirection = STOP;
}

void DCMotor::setDirection(DCMotorDirection direction) {
  if (direction == LEFT || direction == RIGHT || direction == STOP) {
    _newDirection = direction;
  }
}

DCMotorDirection DCMotor::getDirection() {
	return _currentDirection;
}

void DCMotor::commit() {
  this->setMotorPWMFreq();
  if (_currentDirection != _newDirection) {
    if (_currentSpeed > 0) {
      this->smoothStop();
	  }
    _currentDirection = _newDirection;
    this->smoothStart();
  }
  if (_currentSpeed != _newSpeed) {
    this->adjustSpeed();
  }
}

void DCMotor::start() {
  _currentDirection = _newDirection;
  _currentSpeed = _newSpeed;
  this->setMotorPWMFreq();
  if (_currentDirection == LEFT) {
    analogWrite(pins[RIGHT], 0);
    analogWrite(pins[_currentDirection], _currentSpeed);
  } else if (_currentDirection == RIGHT) {
    analogWrite(pins[LEFT], 0);
    analogWrite(pins[_currentDirection], _currentSpeed);
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

void DCMotor::stop() {
  analogWrite(pins[RIGHT], 0);
  analogWrite(pins[LEFT], 0);
  _currentSpeed = 0;
}

/* 
 * Reverses direction of a running motor. If motor is stopped, 
 * only direction is reversed, but motor is left stopped.
 */
void DCMotor::reverse() {
  if (_currentSpeed == 0) {
    return;
  }
  this->setMotorPWMFreq();
  this->smoothStop();
  if (_currentDirection == RIGHT) {
    _currentDirection = LEFT;
  } else if (_currentDirection == LEFT) {
    _currentDirection = RIGHT;
  } else {
    return;
  }
  this->smoothStart();
}

void DCMotor::smoothStop() {
  int counter = DCMotorDelay;
  int pwm = _currentSpeed;
  this->setMotorPWMFreq();
  while(pwm > 0) {
    analogWrite(pins[_currentDirection], --pwm);
    if (counter > 0){
      counter--;
    } else {
      delay(1);
      counter = DCMotorDelay;
    }
  }
  _currentSpeed = 0;
}

void DCMotor::adjustSpeed() {
  this->setMotorPWMFreq();
  if (_currentDirection != _newDirection) {
    this->smoothStop();
	_currentDirection = _newDirection;
    this->smoothStart();
	return;
  }
  if (_currentSpeed == _newSpeed) {
    return;
  } else if (_currentSpeed < _newSpeed) {
    this->smoothStart();
  } else {
    volatile int curpwm = _currentSpeed;
    volatile int counter = DCMotorDelay;
    int pwm = _newSpeed;
    while(curpwm > pwm) {
      analogWrite(pins[_currentDirection], --curpwm);
      if (counter > 0){
        counter--;
      } else {
        delay(1);
        counter = DCMotorDelay;
      }
    }
    _currentSpeed = _newSpeed;
  }
}

void DCMotor::smoothStart() {
  volatile int curpwm = _currentSpeed;
  volatile int counter = DCMotorDelay;
  int pwm = _newSpeed;
  this->setMotorPWMFreq();
  while(curpwm < pwm) {
    analogWrite(pins[_currentDirection], ++curpwm);
    if (counter > 0){
      counter--;
    } else {
      delay(1);
      counter = DCMotorDelay;
    }
  }
  _currentSpeed = _newSpeed;
}

// Sets frequency good for most silent motor control
inline void DCMotor::setMotorPWMFreq() {
  TCCR1B = (TCCR1B & (0b11111000 | 0b00000100));    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR1B = (TCCR1B & (B11111000 | B00000101));    // set timer 1 divisor to  1024 for PWM frequency of    30.64 Hz
  //TCCR1B = (TCCR1B & (B11111000 | B00000001));    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR1B = (TCCR1B & (B11111000 | B00000010));    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
}
