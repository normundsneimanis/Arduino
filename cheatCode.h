#ifndef cheatCode_h
#define cheatCode_h

#ifndef TEST
#include "Arduino.h"
#endif

/*
 * Cheat Code design for camera slider
 * enter cheat codes by inputting colors to this class
 * when cheat code is activated:
 * 		Faster type (wait time decrease)
 * 			min wait time is selected
 * 		Slower type (wait time increase)
 * 			max wait time is selected
 * 	step is 1 second for both
 */

#define CC_SLOW_DEFAULT_WAIT_TIME 10
#define CC_FAST_DEFAULT_WAIT_TIME 1 

enum cheatCodeColors {
	CC_OFF,
	CC_RED,
	CC_GREEN,
	CC_BLUE,
};

enum cheatCodeFasterStates {
	CCFS_Initial,
	CCFS_Green1,
	CCFS_Green2,
	CCFS_Green3,
	CCFS_Blue1,
	CCFS_Green4,
	CCFS_Green5,
};

enum cheatCodeSlowerStates {
	CCSS_Initial,
	CCSS_Blue1,
	CCSS_Blue2,
	CCSS_Blue3,
	CCSS_Green1,
	CCSS_Blue4,
	CCSS_Blue5,
};



enum cheatCodeTypes {
	CCT_OFF,
	CCT_SLOWER,
	CCT_FASTER,
};

#define CC_MAX_LEN 6

typedef cheatCodeSlowerStates cheatCodeSlowerState;
typedef cheatCodeFasterStates cheatCodeFasterState;
typedef cheatCodeTypes cheatCodeType;
typedef cheatCodeColors cheatCodeColor;


class cheatCode
{
  public:
    cheatCode();
    void enterKey(cheatCodeColor color);
    cheatCodeType getMode();
    void setRate(int time);
    int getRate();
    void setWaitTime(int waitTime);
    int getWaitTime();
    byte notConfigured();
  private:
    cheatCodeFasterState _fasterState;
    cheatCodeSlowerState _slowerState;
    byte _fasterStateStarted;
    byte _slowerStateStarted;
    byte _rate;
    byte _waitTime;
    cheatCodeColor _codesEntered[CC_MAX_LEN];
    int _codesEnteredIndex;
    cheatCodeColor _sortedCodesEntered[CC_MAX_LEN];
    
};


#endif
