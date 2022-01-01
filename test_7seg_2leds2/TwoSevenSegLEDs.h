#ifndef TwoSevenSegLEDs_h
#define TwoSevenSegLEDs_h

#include "Arduino.h"

class TwoSevenSegLEDs
{
  public:
    TwoSevenSegLEDs(int pin1, int pin2, int resetpin);
    void setLed(int number);
    void init();
  private:
    void clock(int pin);
    void resetLed(void);
    byte _clockPin;
    byte _clockPin2;
    byte _clockResetPin;
    byte _previousNumber;
};


#endif
