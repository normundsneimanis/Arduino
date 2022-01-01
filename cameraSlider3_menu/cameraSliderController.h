#ifndef cameraSliderController_h
#define cameraSliderController_h

#ifndef TEST
#include "Arduino.h"
#else
#include "mockArduino.h"
#endif
// including for IN_* DCM_* typedefs
#include "userInput.h"
#include "HBridgeMotor.h"

class cameraSliderController
{
  public:
	cameraSliderController();
	HBridgeMotorDirection getDirection();
	void setUserInput(userInputDirection direction);
	void setRepeatEnable(int repeatEnable);
	void sliderLeftReached(int leftReached);
	void sliderRightReached(int rightReached);
	void finish(); // clears repeatInProgress
	void init();
#ifdef TEST
	void printState();
#endif

  private:
	userInputDirection _direction;
	bool _repeatEnable;
	bool _leftReached;
	bool _rightReached;
	bool _repeatInProgress;
};


#endif
