#include "cheatCode.h"


cheatCode::cheatCode() {
	_fasterState = CCFS_Initial;
	_slowerState = CCSS_Initial;
    _fasterStateStarted = 0;
    _slowerStateStarted = 0;
    _waitTime = 0;
    _rate = 0;
    for (int i = 0; i < CC_MAX_LEN; i++) {
    	_codesEntered[i] = CC_OFF;
    }
    _codesEnteredIndex = 0;
}

byte cheatCode::notConfigured() {
	return (!(_waitTime && _rate));
}

cheatCodeType cheatCode::getMode() {
	if (_fasterState == CCFS_Green5)
		return CCT_FASTER;
	else if (_slowerState == CCSS_Blue5)
		return CCT_SLOWER;
	else
		return CCT_OFF;
}

void cheatCode::enterKey(cheatCodeColor color) {
	if (_fasterState == CCFS_Green5)
		return;
	else if (_slowerState == CCSS_Blue5)
		return;

	_codesEntered[_codesEnteredIndex++] = color;
	if (_codesEnteredIndex == CC_MAX_LEN) {
		_codesEnteredIndex = 0;
	}

	byte codeStart = _codesEnteredIndex;

	for (int i = 0; i < CC_MAX_LEN; i++) {
		if (codeStart == CC_MAX_LEN) {
			codeStart = 0;
		}
		_sortedCodesEntered[i] = _codesEntered[codeStart++];
	}

//  Serial.println(F("CheatCodes sorted array: "));
//	for (int j = 0; j < CC_MAX_LEN; j++) {
//		Serial.print(j);
//		Serial.print(F(": "));
//		Serial.print(_sortedCodesEntered[j]);
//    Serial.print(F("\t"));
//	}
//    Serial.println(F(""));

	if (_sortedCodesEntered[0] == CC_BLUE && _sortedCodesEntered[1] == CC_BLUE && _sortedCodesEntered[2] == CC_BLUE &&
			_sortedCodesEntered[3] == CC_GREEN && _sortedCodesEntered[4] == CC_BLUE && _sortedCodesEntered[5] == CC_BLUE) {
		_slowerState = CCSS_Blue5;
	} else if (_sortedCodesEntered[0] == CC_GREEN && _sortedCodesEntered[1] == CC_GREEN && _sortedCodesEntered[2] == CC_GREEN &&
			_sortedCodesEntered[3] == CC_BLUE && _sortedCodesEntered[4] == CC_GREEN && _sortedCodesEntered[5] == CC_GREEN){
		_fasterState = CCFS_Green5;
	}
}

void cheatCode::setRate(int time) {
	_rate = time;
}

int cheatCode::getRate() {
	return _rate;
}

void cheatCode::setWaitTime(int waitTime) {
	_waitTime = waitTime;
}

int cheatCode::getWaitTime() {
	return _waitTime;
}

