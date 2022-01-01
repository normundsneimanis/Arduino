#include "mockArduino.h"
#include <string>
#include <iostream>

mockPinMode pinModeData[MAX_PORTS];
int digitalPortData[MAX_PORTS];
int analogPortData[MAX_PORTS];
int arduDelay;
int analogPortWritingsIndex;
int analogPortWritings[MAX_PORTS][MAX_SAVE];
int digitalPortWritingsIndex;
int digitalPortWritings[MAX_PORTS][MAX_SAVE];

void initializeMockArduino() {
	//cout << "Initializing mockArduino\n";
	for (int i = 0; i < MAX_PORTS; i++) {
		pinModeData[i] = PM_UNINITIALIZED;
		digitalPortData[i] = 0;
		analogPortData[i] = 0;
	}
	clearPortWritings();
}

void clearPortWritings() {
	for (int j = 0; j < MAX_PORTS; j++) {
		for (int i = 0; i < MAX_SAVE; i++) {
			digitalPortWritings[j][i] = 0;
			analogPortWritings[j][i] = 0;
		}
	}
	analogPortWritingsIndex = 0;
	digitalPortWritingsIndex = 0;
}

int testNoParallel(int array[][MAX_SAVE], int pina, int pinb) {
	for (int i = 0; i < MAX_SAVE; i++) {
		if (array[pina][i] > 0 && array[pinb][i] > 0) {
			cout << "Both motors spinning at position " + to_string(i) + "\n";
			cout << "Array for pin A:\n";
			printArray(array[pina]);
			cout << "Array for pin B:\n";
			printArray(array[pinb]);
			return 0;
		}
	}
	return 1;
}

int testGraduality(int array[], int startingIndex, int from, int to) {
	int i = startingIndex;
	int j = from;
	while (j <= to) {
		if (array[i] != j) {
			cout << "Graduality incorrect. Id: " + std::to_string(i) + " content: " + std::to_string(array[i]) + " expected " + std::to_string(j) + "\n";
			printArray(array);
			return 0;
		}
		j++;
		i++;
	}
	return 1;
}

void printArray(int array[]) {
	int z = 0;
	while (z < MAX_SAVE) {
		cout << to_string(z) + ": " + to_string(array[z]) + "\t";
		if ((z % 5) == 0)
			cout << "\n";
		z++;
	}
}

void pinMode(int pin, mockPinMode mode) {
	pinModeData[pin] = mode;
}

void analogWrite(int pin, int value) {
	//cout << "Analog write to pin " + std::to_string(pin) + " value " + std::to_string(value) + " portindex " + to_string(analogPortWritingsIndex) + "\n";
	analogPortWritings[pin][analogPortWritingsIndex++] = value;
	if (analogPortWritingsIndex == MAX_SAVE - 1)
		analogPortWritingsIndex = 0;
	analogPortData[pin] = value;
}

void digitalWrite(int pin, int value) {
	digitalPortWritings[pin][digitalPortWritingsIndex++] = value;
	if (digitalPortWritingsIndex == MAX_SAVE - 1)
		digitalPortWritingsIndex = 0;
	digitalPortData[pin] = value;
}

void delay(int del) {
	arduDelay = del;
}

int TCCR1B = 1;
