#include "mockArduino.h"
#include <string>
#include <iostream>

mockPinMode pinModeData[20];
int digitalPortData[20];
int analogPortData[20];
int arduDelay;
int analogPortWritingsIndex = 0;
int analogPortWritings[MAX_SAVE];
int digitalPortWritingsIndex = 0;
int digitalPortWritings[MAX_SAVE];

void initializeMockArduino() {
	//cout << "Initializing mockArduino\n";
	for (int i = 0; i < 20; i++) {
		pinModeData[i] = PM_UNINITIALIZED;
		digitalPortData[i] = 0;
		analogPortData[i] = 0;
	}
	clearPortWritings();
}

void clearPortWritings() {
	for (int i = 0; i < MAX_SAVE; i++) {
		digitalPortWritings[i] = 0;
		analogPortWritings[i] = 0;
	}
	analogPortWritingsIndex = 0;
	digitalPortWritingsIndex = 0;
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
	analogPortWritings[analogPortWritingsIndex++] = value;
	if (analogPortWritingsIndex == MAX_SAVE - 1)
		analogPortWritingsIndex = 0;
	analogPortData[pin] = value;
}

void digitalWrite(int pin, int value) {
	digitalPortWritings[digitalPortWritingsIndex++] = value;
	if (digitalPortWritingsIndex == MAX_SAVE - 1)
		digitalPortWritingsIndex = 0;
	digitalPortData[pin] = value;
}

void delay(int del) {
	arduDelay = del;
}

int TCCR1B = 1;
