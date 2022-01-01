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

const char * const userInputDirectionString[] = {
  "UI_LEFT",
  "UI_RIGHT",
  "UI_STOP",
};

typedef userInputDirection userInputDirection;

class userInput {
  public:
	userInput(int motorDirectionLeftPin, int motorDirectionRightPin);
	void readSensors();
	userInputDirection getDirection();
	void init();

  private:
    uint8_t _motorDirectionLeftPin;
    uint8_t _motorDirectionRightPin;
    uint8_t _motorDirectionLeftReading;
    uint8_t _motorDirectionRightReading;
};


#endif
