#include "Arduino.h"
#include "TimeLapse.h"

TimeLapse::TimeLapse(int TLWaitTimeSaveAddr, int TLRunTimeSaveAddr, int TLIncreasingRateSaveAddr, int TLDecreasingRateSaveAddr) {
  _TLWaitTimeSaveAddr = TLWaitTimeSaveAddr;
  _TLRunTimeSaveAddr = TLRunTimeSaveAddr;
  _TLIncreasingRateSaveAddr = TLIncreasingRateSaveAddr;
  _TLDecreasingRateSaveAddr = TLDecreasingRateSaveAddr;
  _picturesTaken = 0;
  _lastButtonTime = 0;
  _configuringWhat = 0;
  _cheatEnabled = 0;
  _cheatConfigured = 0;
  _timeLapseType = TLProgressiveOff;
  this->loadIncreasingTime();
  this->loadDecreasingTime();
}

void TimeLapse::init() {
  // nothing to be done here
}

// Read Time Lapse wait time from memory on startup
int TimeLapse::eepromReadWait() {
  int timeLapseWait = EEPROM.read(_TLWaitTimeSaveAddr);
  if (timeLapseWait < TLMinWaitTime || timeLapseWait > TLMaxWaitTime) {
    timeLapseWait = TLDefaultWaitTime;
    EEPROM.write(_TLWaitTimeSaveAddr, timeLapseWait);
  }
  return timeLapseWait;
}

void TimeLapse::eepromWriteWait() {
  EEPROM.write(_TLWaitTimeSaveAddr, _TLWaitTime);
}

// Read Time Lapse wait time from memory on startup
int TimeLapse::eepromReadRun() {
  int timeLapseRun = EEPROM.read(_TLRunTimeSaveAddr);
  if (timeLapseRun < TLMinRunTime || timeLapseRun > TLMaxRunTime) {
    timeLapseRun = TLDefaultRunTime;
    EEPROM.write(_TLRunTimeSaveAddr, timeLapseRun);
  }
  return timeLapseRun;
}

void TimeLapse::eepromWriteRun() {
  EEPROM.write(_TLRunTimeSaveAddr, _TLRunTime);
}

int TimeLapse::setWaitTime(int waitTime) {
  _TLWaitTime = waitTime;
  return _TLWaitTime;
}

int TimeLapse::setRunTime(int runTime) {
  if (runTime > TLMaxRunTime) {
    _TLRunTime = TLMaxWaitTime;
  } else if (runTime < TLMinRunTime) {
    _TLRunTime = TLMinRunTime;
  } else {
    _TLRunTime = runTime;
  }
  return _TLRunTime;
}

// returns what config state should happen
int TimeLapse::configButtonPressed() {
	if ((millis() - _lastButtonTime) < 2000) {
		_cheatState++;
		if (_cheatConfigured) {
			_cheatConfigured = 0;
		}
		if (_cheatState > 2) {
			_cheatEnabled = 1;
		}
	} else {
		_cheatState = 0;
	}

	_lastButtonTime = millis();

	_configuringWhat++;
	if (_cheatEnabled && _timeLapseType == TLProgressiveOff) {
		_configuringWhat = 3;
	} else if (_cheatEnabled && !_cheatConfigured) {
		_configuringWhat = 4;
	} else {
		if (_configuringWhat > 2) {
			_configuringWhat = 0;
		}
	}

	return _configuringWhat;
}

int TimeLapse::cheatEnabled() {
	return _cheatEnabled;
}

int TimeLapse::cheatConfigured() {
	return _cheatConfigured;
}

void TimeLapse::setCheatType(timeLapseType type) {
	_timeLapseType = type;
}

int TimeLapse::getCheatType() {
	return _timeLapseType;
}

void TimeLapse::saveIncreasingTime() {
	return EEPROM.write(_TLIncreasingRateSaveAddr, _cheatIncreasingRate);
}

void TimeLapse::loadIncreasingTime() {
	_cheatIncreasingRate = EEPROM.read(_TLIncreasingRateSaveAddr);
}

int TimeLapse::getIncreasingTime() {
	return _cheatIncreasingRate;
}

void TimeLapse::setIncreasingTime(int time) {
	_cheatIncreasingRate = time;
	_cheatConfigured = 1;
	this->saveIncreasingTime();
}

void TimeLapse::saveDecreasingTime() {
	return EEPROM.write(_TLDecreasingRateSaveAddr, _cheatDecreasingRate);
}

void TimeLapse::loadDecreasingTime() {
	_cheatDecreasingRate = EEPROM.read(_TLDecreasingRateSaveAddr);
}

int TimeLapse::getDecreasingTime() {
	return _cheatDecreasingRate;
}

void TimeLapse::setDecreasingTime(int time) {
	_cheatDecreasingRate = time;
	_cheatConfigured = 1;
	this->saveDecreasingTime();
}

void TimeLapse::waitForCamera(int waitTime) {
  if (waitTime > 0) {
    delay(waitTime);
  }
}

void TimeLapse::waitForButton() {
  delay(TLCameraButtonPressTime);
}

void TimeLapse::pictureTaken() {
  _picturesTaken++;
}
int TimeLapse::getPicturesTaken() {
  return _picturesTaken;
}
void TimeLapse::clearPicturesTaken() {
  _picturesTaken = 0;
}

