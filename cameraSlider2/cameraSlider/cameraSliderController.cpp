#include "cameraSliderController.h"
#ifdef TEST
#include <iostream>
#include <string>
using namespace std;
#endif

cameraSliderController::cameraSliderController() {
	_direction = UI_STOP;
	_repeatEnable = 0;
	_leftReached = 0;
	_rightReached = 0;
	_repeatInProgress = 0;
}

void cameraSliderController::init() {
	// Nothing to be done here
}

HBridgeMotorDirection cameraSliderController::getDirection() {
	if (!_repeatEnable) {
		if (_direction == UI_RIGHT && (!_rightReached)) {
			return HBM_RIGHT;
		} else if (_direction == UI_LEFT && (!_leftReached)) {
			return HBM_LEFT;
		} else {
			return HBM_STOP;
		}
	} else {
		// right, left sensor on, right sensor off
		// right, repeatInProgress, right sensor on
		// left, right sensor on, left sensor off
		// left, repeatInProgress, left sensor on
		if (!_repeatInProgress) {
			if (_direction == UI_RIGHT) {
				if (!_rightReached) {
					return HBM_RIGHT;
				} else {
					_repeatInProgress = 1;
					return HBM_LEFT;
				}
			} else if (_direction == UI_LEFT) {
				if (!_leftReached) {
					return HBM_LEFT;
				} else {
					_repeatInProgress = 1;
					return HBM_RIGHT;
				}
			} else {
				return HBM_STOP;
			}
		} else {
			if (_direction == UI_LEFT) {
				if (!_rightReached) {
					return HBM_RIGHT;
				} else {
					_repeatInProgress = 0;
					return HBM_LEFT;
				}
			} else if (_direction == UI_RIGHT) {
				if (!_leftReached) {
					return HBM_LEFT;
				} else {
					_repeatInProgress = 0;
					return HBM_RIGHT;
				}
			} else {
				//_repeatInProgress = 0;
				return HBM_STOP;
			}
		}
	}
}

#ifdef TEST
void cameraSliderController::printState() {
	cout << "cameraSliderController status:\n";
	cout << "_direction: " + to_string(_direction) + "\n";
	cout << "_repeatEnable: " + to_string(_repeatEnable) + "\n";
	cout << "_leftReached: " + to_string(_leftReached) + "\n";
	cout << "_rightReached: " + to_string(_rightReached) + "\n";
	cout << "_repeatInProgress: " + to_string(_repeatInProgress) + "\n";
}
#endif

void cameraSliderController::setUserInput(userInputDirection direction) {
	_direction = direction;
	if (_direction == UI_STOP) {
		_repeatInProgress = 0;
	}
}

void cameraSliderController::setRepeatEnable(int repeatEnable) {
	_repeatEnable = repeatEnable;
}

void cameraSliderController::sliderLeftReached(int leftReached) {
	_leftReached = leftReached;
}

void cameraSliderController::sliderRightReached(int rightReached) {
	_rightReached = rightReached;
}

void cameraSliderController::finish() {
	_repeatInProgress = 0;
}
