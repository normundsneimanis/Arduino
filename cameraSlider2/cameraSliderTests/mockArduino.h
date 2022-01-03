#ifndef mockArduino_h
#define mockArduino_h
#include <stdint.h>
using namespace std;

typedef uint8_t byte;

// max ports that can be tested
#define MAX_PORTS 20
// max values per port before its overflowing
#define MAX_SAVE 2048

enum mockPinMode {
	PM_UNINITIALIZED,
	INPUT,
	OUTPUT,
};

// #define F(expr) expr

class SerialClass {
public:
	SerialClass();
	void print(const char array[]);
	void print(int value);
	void println(const char array[]);
	void println(int value);
};

typedef mockPinMode mockPinMode;

void initializeMockArduino();
void clearPortWritings();
int testNoOther(int pin);
int testNoOther(int pina, int pinb);
int testNoParallel(int array[][MAX_SAVE], int pina, int pinb);
int testGraduality(int array[], int startingIndex, int from, int to);
void printArray(int array[]);
void pinMode(int pin, mockPinMode mode);
void analogWrite(int pin, int value);
void digitalWrite(int pin, int value);
void delay(int del);
const char * F(const char array[]);

#endif
