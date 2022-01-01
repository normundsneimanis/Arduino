#include "Arduino.h"
#include "timedelay.h"

timedelay::timedelay(int delayTime) {
  _previousTime = 0;
  _delayTime = delayTime;
}
bool timedelay::hasPassed() {
  if (_currentTime + _delayTime > _previousTime) {
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

