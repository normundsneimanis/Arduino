#ifndef DCMotor_h
#define DCMotor_h

#ifndef TEST
#include "Arduino.h"
#else
#include "mockArduino.h"
#endif

enum DCMotorDirection {
  RIGHT,
  LEFT,
  STOP,
};

typedef DCMotorDirection DCMotorDirection;

class DCMotor
{
  public:
    DCMotor(int leftPin, int rightPin);
    void setDirection(DCMotorDirection Direction);
    void setSpeed(int speed);
    void commit();
    void reverse();
    void stop();
    void start(); // Starts motor without gradually increasing its speed
    void smoothStop();
    void smoothStart();
    void adjustSpeed();
	DCMotorDirection getDirection();
  private:
    void setMotorPWMFreq();
    int pins[2];
    DCMotorDirection _currentDirection;
    DCMotorDirection _newDirection;
    int _currentSpeed;
    int _newSpeed;
    const int DCMotorDelay = 1;
};


#endif
