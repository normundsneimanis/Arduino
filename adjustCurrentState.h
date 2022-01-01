#ifndef adjustCurrentState_h
#define adjustCurrentState_h

#ifndef TEST
#include "Arduino.h"
#endif

class adjustCurrentState
{
  public:
    adjustCurrentState(int currentPot, int currentVal, int min, int max);
    int update(int currentPot);
    void setScaleMin(int scaleMin);
    void setScaleMax(int scaleMax);
    int getCurrent();
  private:
    void _setBoundaries();
    int _currentPot;
    int _currentVal;
    int _min;
	int _max;
	int _scaleMin;
	int _scaleMax;
	int _boundaryMin;
	int _boundaryMax;
};


#endif
