/*
 * Class usage:
 * rotaryEncoder encoder(pinLeft, pinRight, buttonPin);
 * 
 * setup():
 *		encoder.init();
 *		attachInterrupt(digitalPinToInterrupt(_pinLeft), rotaryEncoderISR, CHANGE);
 *
 *	ISR: copy function and follow instructions
 */

// Copy this to the code and change pinLeft, pinRight and coder to your encoder instance
// also add this to setup()   attachInterrupt(digitalPinToInterrupt(_pinLeft), rotaryEncoderISR, CHANGE); 
//void rotaryEncoderISR() {
//  if (digitalRead(_pinLeft) == digitalRead(_pinRight)) {
//    coder.stepUp();
//  } else {
//    coder.stepDown();
//  }
//}

#ifndef rotaryEncoder_h
#define rotaryEncoder_h

#include "Arduino.h"
#include <limits.h>

class rotaryEncoder
{
public:
	rotaryEncoder(int pinLeft, int pinRight, int buttonPin);
	void init();
	void setStep(int step);
	void setCurrentPosition(int position);
  void setPushTime(int tm);
  int buttonLongPressed();
  int buttonPressedReleased();
	int getCurrentPosition();
	int buttonPressed();
	void stepUp();
	void stepDown();
	void setLimits(int min, int max);
private:
	byte _buttonState;
	int _currentPosition;
	int _step;
	int _pinLeft;
	int _pinRight;
	int _buttonPin;
	int _min;
	int _max;
  int _pushTime;
  unsigned long _pusStartTime;
};


#endif
