#ifndef DCMotor_h
#define DCMotor_h
#ifdef TEST
#include "mockArduino.h"
#else
#include "Arduino.h"
#endif

// Delay between writes to analog port when slowing down gradually.
// This number is to be determined experimentally
#define DCMotorDelay 1

class DCMotor {
public:
	DCMotor(int pin);
	void setSpeed(int speed);
	void commit();
	void stop();
	void start(); // Starts motor without gradually increasing its speed
	void smoothStop();
	void smoothStart();
	void adjustSpeed();
	void setSmooth(byte smooth);
	void init();
private:
	void _smoothStop(int speed); // lowers speed to the one given in argument
	int _pin;
	int _currentSpeed;
	int _newSpeed;
	byte _smooth;
};

#endif
