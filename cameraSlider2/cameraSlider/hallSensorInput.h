#ifndef hallSensorInput_h
#define hallSensorInput_h

#include "Arduino.h"

class hallSensorInput
{
  public:
	hallSensorInput(int leftPin, int rightPin);
	void readSensors();
	int endReached();
	int rightReached();
	int leftReached();
  int getRawLeft();
  int getRawRight();
	void init();
  private:
	int _rightPin;
	int _leftPin;
	int _leftValue;
	int _rightValue;
};

#endif
