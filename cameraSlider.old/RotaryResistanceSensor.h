#ifndef RotaryResistanceSensor_h
#define RotaryResistanceSensor_h

#include "Arduino.h"

class RotaryResistanceSensor {
  public:
    RotaryResistanceSensor(int pin);
    int readSensor();
    int ledValue();
    int normalizeValue();
  private:
    int _pin;
    int _rawReading;
    int _normalizedReading;
    float _ledReading;
    int _sensorValuePrev;
};


#endif
