#ifndef HBridgeMotor_h
#define HBridgeMotor_h
#include "DCMotor.h"

#ifndef TEST
#include "Arduino.h"
#include "userInput.h"
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
    HBridgeMotor(uint8_t pwmPin, uint8_t leftPin, uint8_t rightPin);
    void setDirection(userInputDirection mdirection);
    void setSpeed(int speed);
    int getSpeed();
    int isRunning();
    void commit();
    void reverse();
    void stop();
    void start(); // Starts motor without gradually increasing its speed
    void init();
    void smoothEnable(byte enable);
	userInputDirection getDirection();
  private:
	DCMotor *_motorLeft;
	DCMotor *_motorRight;
    void _setMotorPWMFreq();
    uint8_t _pins[2];
    uint8_t _pwmPin;
    userInputDirection _direction;
    uint8_t _speed;
    uint8_t _smoothEnable;
};


#endif
