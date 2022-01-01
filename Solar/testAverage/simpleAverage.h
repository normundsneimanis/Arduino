#ifndef userInput_h
#define userInput_h

#ifndef TEST
#include "Arduino.h"
#endif
#include <string.h>

class average {
  public:
	average(size_t numElements);
	void add(int num);
	int getAverage();
	int count();

  private:
	size_t _numElements;
	int * _array;
	unsigned int _currentElement;
};


#endif
