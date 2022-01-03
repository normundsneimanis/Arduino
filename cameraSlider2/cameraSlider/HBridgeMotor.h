#ifndef HBridgeMotor_h
#define HBridgeMotor_h
#include "DCMotor.h"

#ifndef TEST
#include "Arduino.h"
#else
#include "mockArduino.h"
#endif

enum HBridgeMotorDirection {
	HBM_LEFT,
	HBM_RIGHT,
	HBM_STOP,
};

//const char *DCM_to_str[] = {
//	"HBM_LEFT",
//	"HBM_RIGHT",
//	"HBM_STOP",
//};

typedef HBridgeMotorDirection HBridgeMotorDirection;

class HBridgeMotor
{
  public:
    HBridgeMotor(int leftPin, int rightPin);
    void setDirection(HBridgeMotorDirection mdirection);
    void setSpeed(int speed);
    int getSpeed();
    int isRunning();
    void commit();
    void reverse();
    void stop();
    void start(); // Starts motor without gradually increasing its speed
    void init();
    void smoothEnable(byte enable);
	HBridgeMotorDirection getDirection();
  private:
	DCMotor *_motorLeft;
	DCMotor *_motorRight;
    void _setMotorPWMFreq();
    int _pins[2];
    HBridgeMotorDirection _direction;
    int _speed;
    int _smoothEnable;
};


#endif
