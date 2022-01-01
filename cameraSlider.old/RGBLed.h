#ifndef RGBLed_h
#define RGBLed_h

#include "Arduino.h"

class RGBLed
{
  public:
    RGBLed(int redPin, int greenPin, int bluePin);
    void yellow();
    void green();
    void blue();
    void red();
    void off();
  private:
    void setLedPWMFreq();
    int _redPin;
    int _greenPin;
    int _bluePin;
};


#endif
