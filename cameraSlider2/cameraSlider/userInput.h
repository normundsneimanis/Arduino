#ifndef userInput_h
#define userInput_h

#ifndef TEST
#include "Arduino.h"
#endif

enum userInputDirection {
	UI_LEFT,
	UI_RIGHT,
	UI_STOP,
};

typedef userInputDirection userInputDirection;

class userInput {
  public:
	userInput(int motorDirectionLeftPin, int motorDirectionRightPin, int repeatPin);
	void readSensors();
	userInputDirection getDirection();
	int repeatEnabled();
	void init();

  private:
    int _motorDirectionLeftPin;
    int _motorDirectionRightPin;
    int _repeatPin;
    int _motorDirectionLeftReading;
    int _motorDirectionRightReading;
    int _repeatReading;
};


#endif
