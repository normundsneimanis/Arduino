#ifndef timedelay_h
#define timedelay_h

#include "Arduino.h"

class timedelay
{
  public:
    timedelay(unsigned long delayTime);
    bool hasPassed();
    void setTime(unsigned long delayTime);
    unsigned long getTime();
    void cycle();
    void reset();
  private:
    unsigned long _delayTime;
    unsigned long  _previousTime;
    unsigned long _currentTime;
    //int _hasCycled; // used so we can use hasPassed after reset() is called.
};


#endif

