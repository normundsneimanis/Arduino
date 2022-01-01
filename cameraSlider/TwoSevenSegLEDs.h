#ifndef TwoSevenSegLEDs_h
#define TwoSevenSegLEDs_h

#include "Arduino.h"

class TwoSevenSegLEDs
{
  public:
    TwoSevenSegLEDs(int pin1, int pin2, int resetpin);
    void setLed(int number);
  private:
    void clock(int pin);
    void resetLed(void);
    int clockPin;
    int clockPin2;
    int clockResetPin;
};


#endif
