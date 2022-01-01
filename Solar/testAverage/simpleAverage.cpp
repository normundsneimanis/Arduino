#ifndef TEST
#include "Arduino.h"
#endif
#include "average.h"

average::average(size_t numElements) {
	_array = (int *) malloc(numElements * sizeof(int));
	if (_array == NULL) {
		Serial.println(F("Failed to initialize array"));
	}
	memset(_array, 0, numElements * sizeof(int));
	_currentElement = 0;
	_numElements = numElements;
}


void average::add(int num) {
	_array[_currentElement++] = num;
	if (_currentElement == _numElements) {
		_currentElement = 0;
	}
}

int average::count() {
	int inserted = 0;
	for (size_t i = 0; i < _numElements; i++) {
		if (_array[i] > 0) {
			inserted++;
		}
	}

	return inserted;
}


int average::getAverage() {
	int average = 0;
	int inserted = 0;
	for (size_t i = 0; i < _numElements; i++) {
		if (_array[i] > 0) {
			average += _array[i];
			inserted++;
		}
	}
	
	return average / inserted;
}
