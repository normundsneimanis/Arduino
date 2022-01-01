#include "Arduino.h"
#include "timedelay.h"

timedelay::timedelay(unsigned long delayTime) {
  _previousTime = 0;
  _delayTime = delayTime;
}

void timedelay::setTime(unsigned long delayTime) {
	_delayTime = delayTime;
}

void timedelay::trigger() {
  _previousTime = _currentTime - _delayTime;
}

unsigned long timedelay::getTime() {
  return _delayTime;
}

bool timedelay::hasPassed() {
  if (_currentTime > _previousTime + _delayTime) {
    return 1;
  } else {
    return 0;
  }
}

void timedelay::reset() {
   _previousTime = _currentTime;
}

void timedelay::cycle() {
  _currentTime = millis();
}

