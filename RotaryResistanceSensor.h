#ifndef RotaryResistanceSensor_h
#define RotaryResistanceSensor_h

#include "Arduino.h"

class RotaryResistanceSensor {
  public:
    RotaryResistanceSensor(int pin);
    void readSensor();
    int getRawReading();
    int getLedValue();
    int getNormalizedValue();
    void init();
  private:
    int _pin;
    int _rawReading;
    int _normalizedReading;
    float _ledReading;
    int _sensorValuePrev;
};


#endif
