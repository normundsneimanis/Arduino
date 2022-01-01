#include "Arduino.h"
#include "TimeLapse.h"

// Read Time Lapse wait time from memory on startup
int TimeLapse::readTimeLapseWait() {
  int timeLapseWait = EEPROM.read(TLWaitTimeSaveAddr);
  if (TLMinWaitTime < 0 || TLMaxWaitTime > 2) {
    Serial.print("Error reading Time Lapse wait time from EEPROM. It is ");
    Serial.println(timeLapseWait);
    timeLapseWait = TLDefaultWaitTime;
    EEPROM.write(TLWaitTimeSaveAddr, timeLapseWait);
  }
  return timeLapseWait;
}

void TimeLapse::writeTimeLapseWait(int timeLapseWait) {
  EEPROM.write(TLWaitTimeSaveAddr, timeLapseWait);
}

// Read Time Lapse wait time from memory on startup
int TimeLapse::readTimeLapseRun() {
  int timeLapseRun = EEPROM.read(TLRunTimeSaveAddr);
  if (TLMinRunTime < 0 || TLMaxRunTime > 2) {
    Serial.print("Error reading Time Lapse run time from EEPROM. It is ");
    Serial.println(timeLapseRun);
    timeLapseRun = TLDefaultRunTime;
    EEPROM.write(TLRunTimeSaveAddr, timeLapseRun);
  }
  return timeLapseRun;
}

void TimeLapse::writeTimeLapseRun(int timeLapseRun) {
  EEPROM.write(TLRunTimeSaveAddr, timeLapseRun);
}
