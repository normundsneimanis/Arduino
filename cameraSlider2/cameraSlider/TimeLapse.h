#ifndef TimeLapse_h
#define TimeLapse_h

#include "Arduino.h"
#include "RGBLed.h"
#include <EEPROM.h>

// Time Lapse settings
 // 2 seconds
#define TLDefaultWaitTime 2
// 0 = 0.5 seconds, 1 = 1 second, to refactor 0.5 we need to change
// adjustCurrentState and add ifs to start secion of TimeLapse
#define TLMinWaitTime 1
// 99 seconds
#define TLMaxWaitTime 99
// 0.025 seconds
#define TLMinRunTime 1
 // 2 seconds
#define TLMaxRunTime 80
// Time Lapse settings save address in EEPROM
// 0.1 seconds
#define TLDefaultRunTime 1
// Lit green led before taking picture for X msec
#define TLGreenTime 10
// camera stabilizes after driving for 200 msec before taking picture
#define TLCameraDelayTime 500
// Time to press button so snapshot is taken - 100 msec
#define TLCameraButtonPressTime 100

enum timeLapseType {
	TLProgressiveOff,
	TLProgressiveIncreasing,
	TLProgressiveDecreasing,
	NumtimeLapseTypes,
};

typedef timeLapseType timeLapseType;

enum timeLapseState {
  TLOff,
  TLWaitingForStart,
  TLMotorRunning,
  TLMotorDone,
  TLDone,
};

typedef timeLapseState timeLapseState;

class TimeLapse
{
  public:
    TimeLapse(int TLWaitTimeSaveAddr, int TLRunTimeSaveAddr, int TLIncreasingRateSaveAddr, int TLDecreasingRateSaveAddr);
    int eepromReadWait();
    void eepromWriteWait();
    int eepromReadRun();
    void eepromWriteRun();
    int configButtonPressed(); // returns what config state should happen
    int cheatEnabled();
    int setWaitTime(int waitTime);
    int setRunTime(int runTime);
    void waitForCamera(int waitTime);
    void waitForButton();
    void pictureTaken();
    int getPicturesTaken();
    void clearPicturesTaken();
    void init();
    void setCheatType(timeLapseType type);
    int getCheatType();
    void saveIncreasingTime();
    void loadIncreasingTime();
    int getIncreasingTime();
    void setIncreasingTime(int time);
    void saveDecreasingTime();
    void loadDecreasingTime();
    int getDecreasingTime();
    void setDecreasingTime(int time);
    int cheatConfigured();
  private:
    int _TLWaitTimeSaveAddr;
    int _TLRunTimeSaveAddr;
    int _TLIncreasingRateSaveAddr;
    int _TLDecreasingRateSaveAddr;
    int _TLWaitTime; // Time to wait between pictures, seconds. 0 means 0.5 seconds
    int _TLRunTime; // For how long to move motor. 1 = 100 milliseconds (tenths of seconds)
    int _picturesTaken;
    int _lastButtonTime;
    int _configuringWhat;
    int _cheatState;
    int _cheatEnabled;
    int _cheatConfigured;
    int _cheatIncreasingRate;
    int _cheatDecreasingRate;
    timeLapseType _timeLapseType;
};


#endif
