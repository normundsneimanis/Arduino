#ifndef simpleAverage_h
#define simpleAverage_h

#ifndef TEST
#include "Arduino.h"
#endif
#include <string.h>

class simpleAverage {
  public:
	simpleAverage(size_t numElements);
	void add(int num);
	int getAverage();
	int count();
  void clear();

  private:
	size_t _numElements;
	int * _array;
	unsigned int _currentElement;
};


#endif
