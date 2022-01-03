#ifndef RGBLed_h
#define RGBLed_h

#include "Arduino.h"

#define RGBLED_PWM 111
#define RGBLED_BLUE 255

class RGBLed
{
  public:
    RGBLed(int redPin, int greenPin, int bluePin);
    void yellow();
    void green();
    void blue();
    void red();
    void off();
    void init();
    void white();
  private:
    int _redPin;
    int _greenPin;
    int _bluePin;
};


#endif
