#include "Arduino.h"
#include "RotaryResistanceSensor.h"

RotaryResistanceSensor::RotaryResistanceSensor(int resistorPin) {
  _pin = resistorPin;
}

void RotaryResistanceSensor::init() {
	pinMode(_pin, INPUT);
}

void RotaryResistanceSensor::readSensor() {
  _rawReading = ((analogRead(_pin) - 1020) * -1);
}

int RotaryResistanceSensor::getRawReading() {
  return _rawReading;
}

// Our value can be 0 to 1024, so divide it by 10 to get something closer to 0 - 99 for showing on LED.
int RotaryResistanceSensor::getLedValue() {
  _ledReading = (float) _rawReading / 10;
  _ledReading += (_ledReading / 15 + 0.5); // This is to add 6 when value is 90, 3 when 50 and 0 when 0
  return (int) _ledReading;
}

int RotaryResistanceSensor::getNormalizedValue() {
  _normalizedReading = (int) _rawReading / 10;
  if (_normalizedReading < 0)
    _normalizedReading = 0;
  if (_normalizedReading > 99) {
    _normalizedReading = 99;
  }

  // We want to do a little reading normalization so our motor speed stays constant upon small sensor changes
  if (_normalizedReading < _sensorValuePrev + 1 && _normalizedReading > _sensorValuePrev - 1) {
    _normalizedReading = _sensorValuePrev;
  } else {
    _sensorValuePrev = _normalizedReading;
  }

  return _normalizedReading;
}
