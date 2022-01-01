#ifndef TimeLapse_h
#define TimeLapse_h

#include "Arduino.h"
#include "RGBLed.h"
#include <EEPROM.h>

// Time Lapse settings
 // 2 seconds
#define TLDefaultWaitTime 2
// 0.5 seconds
#define TLMinWaitTime 0
// 99 seconds
#define TLMaxWaitTime 99
// 0.1 second
#define TLMinRunTime 1
 // 2 seconds
#define TLMaxRunTime 20
// Time Lapse settings save address in EEPROM
// 0.5 seconds
#define TLDefaultRunTime 5
#define TLWaitTimeSaveAddr 1
#define TLRunTimeSaveAddr 2

class TimeLapse
{
  public:
    TimeLapse(int waitConfigPin, int runConfigPin, int bluePin);
  private:
    int readTimeLapseWait();
    void writeTimeLapseWait(int timeLapseWait);
    int readTimeLapseRun();
    void writeTimeLapseRun(int timeLapseRun);
    int TLWaitTime = -1; // Time to wait between pictures, seconds. 0 means 0.5 seconds
    int TLRunTime = 0; // For how long to move motor. 1 = 100 milliseconds (tenths of seconds)
};


#endif
