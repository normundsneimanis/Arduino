#ifndef DCMotorTest_h
#define DCMotorTest_h
using namespace std;

// max ports that can be tested
#define MAX_PORTS 20
// max values per port before its overflowing
#define MAX_SAVE 2048

enum mockPinMode {
	PM_UNINITIALIZED,
	INPUT,
	OUTPUT,
};

typedef mockPinMode mockPinMode;

void initializeMockArduino();
void clearPortWritings();
int testNoParallel(int array[][MAX_SAVE], int pina, int pinb);
int testGraduality(int array[], int startingIndex, int from, int to);
void printArray(int array[]);
void pinMode(int pin, mockPinMode mode);
void analogWrite(int pin, int value);
void digitalWrite(int pin, int value);
void delay(int del);

#endif
