#include "adjustCurrentState.h"
#include <stdlib.h>

adjustCurrentState::adjustCurrentState(int currentPot, int currentVal, int min, int max) {
	_currentPot = currentPot;
	_currentVal = currentVal;
	_min = min;
	_max = max;
	_scaleMin = 0;
	_scaleMax = 99;
	this->_setBoundaries();
}

int adjustCurrentState::getCurrent() {
	if (_currentVal > _max) {
		_currentVal = _max;
	} else if (_currentVal < _min) {
		_currentVal = _min;
	}
	if (_currentVal + _max - _currentVal > _scaleMax) {
		_currentVal -= _currentVal + _max - _currentVal - _scaleMax;
	} else if (_currentPot - _currentVal < _scaleMin) {
		_currentVal += _currentPot - _currentVal - _scaleMin;
	}

	return _currentVal;
}

void adjustCurrentState::_setBoundaries() {
	if (_currentVal + _max - _currentVal > _scaleMax) {
		_currentVal -= _currentVal + _max - _currentVal - _scaleMax;
	} else if (_currentPot - _currentVal < _scaleMin) {
		_currentVal += _currentPot - _currentVal - _scaleMin;
	}

//	if (_currentPot > _scaleMax || _currentPot + _max > _scaleMax)
//		_boundaryMax = _max;
//		_boundaryMin = _boundaryMax - _max;
//	if (_currentPot < _scaleMin || _currentPot - _max < _scaleMin) {
//		_boundaryMin = _min;
//		_boundaryMax = _boundaryMin + _max;
//	} else {
//		_boundaryMin = _currentPot - (_max - _min / 2);
//		_boundaryMax = _boundaryMin + _max + _min;
//	}
}

int adjustCurrentState::update(int currentReading) {
	if (currentReading - _boundaryMin < _min) {
		return _min;
	} else {
		return currentReading - _boundaryMin;
	}
//	// (1, 5, 1, 20) == 1
//	// (3, 5, 1, 20) == 3
//	if (reading - currentState < scaleMin) {
//		return reading;
//	} else if (reading + (max - currentState) > scaleMax) {
//		//(84, 15, 1, 20) = 15
//		//(90, 15, 1, 20) = 11
//		//(85, 15, 1, 20) = 14
//		return currentState - (scaleMax - (reading + (max - currentState)));
//	}
//	return currentState;
}

void adjustCurrentState::setScaleMin(int scaleMin) {
	_scaleMin = scaleMin;
}
void adjustCurrentState::setScaleMax(int scaleMax) {
	_scaleMax = scaleMax;
}

//// Adjust current state so it fits our rotary potentiometer as good as possible
//int adjustCurrentState(int reading, int currentState, int min, int max) {
//
//}
