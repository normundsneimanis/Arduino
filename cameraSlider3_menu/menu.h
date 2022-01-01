#ifndef menu_h
#define menu_h

#include "Arduino.h"
#include "rotaryEncoder.h"
#include "images.h"
#include "userInput.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <Fonts/MyriadProRegular10pt7b.h>
#include <Fonts/MyriadProRegular7pt7b.h>
#include <Fonts/MyriadProRegular28pt7bNum.h>
#include <SPI.h>

#define cs_lcd   4
#define dc       3
#define rst      2

// slider run time information, changed from interrupt
typedef struct {
  uint16_t picsTaken;
  uint16_t nextFrameTime; // Time left till next frame, 10ths of second
  uint16_t framesLeft;
  uint32_t endTime;
  uint8_t repeat;
  uint32_t startTimeLeft;
} runTimeInfo_t;

// slider configuration information that does not change in interrupt
typedef struct {
  uint8_t sliderMode;
  uint16_t frames; // for timelapse
  uint16_t timeBetweenFrames;
  int8_t motorWaitIncreaseStep;
  uint8_t motorStep;
  uint8_t motorStepMultiplier;
  uint16_t waitBeforePic;
  uint8_t picReleaseTime;
  uint8_t speed;
  uint32_t startTime; // millis() of Start time of slider
  uint32_t endTime; // millis() of end time of slider
} runTimeConfig_t;

void printRunTimeConfig(runTimeConfig_t * rc);
void printRunTimeInfo(runTimeInfo_t * ri);
void printRunTimeInfo(volatile runTimeInfo_t * ri);

class menu
{
  public:
    menu(Adafruit_ST7735 &tft, rotaryEncoder &encoder, userInput &userInput);
    void init();
    uint8_t updateEncoder(); // returns 1 if screen must be updated
    void infoUpdated();
    void sliderFinished();
    void draw();
    void picsLeftAndTotal();
    void nextPic();
    void timeLeft();
    void picsTaken();
    void videoLength();
    int limitMinCB();
    int limitMaxCB();
    int getCurrentPosition();
    int currentPositionCB();
    void calcTLPro();
    void updateBatteryStatus(uint16_t value);
    void startInFinished();

    void modeSelectDraw();
    void modeWarningDraw();
    void modeMenuDraw();
    void menuChangeDraw();
    void waitRunDraw();
    void runDraw();
    void viewConfigDraw();
    void finishedDraw();
    void finishedDirWDraw();
    void modeSelectEncoder(int16_t value);
    void modeWarningEncoder(int16_t value);
    void modeMenuEncoder(int16_t value);
    void menuChangeEncoder(int16_t value);
    void runEncoder(int16_t value);
    void viewConfigEncoder(int16_t value);
    void modeSelectButton();
    void modeWarningButton();
    void modeMenuButton();
    void menuChangeButton();
    void runButton();
    void viewConfigButton();
    void finishedButton();
    void longButton();
    void waitRunDirOff();
    void runDirOff();
    void runDirOn();
    void viewConfigDirOff();
    void finishedDirOff();
    void finishedDirWOff();
    void modeWarningDirOn();
    void modeWarningDirOff();
    void modeMenuDirOn();
    void inputUpdate();
    void calculate();

    void _stopMoDelaySet(int);
    void _stopMoDelayPrint();
    int _limitMinZero();
    int _limitMinOne();
    int _stopMoDelayMax();
    int _stopMoDelayCurrent();
    void _stopMoStepSet(int);
    void _stopMoStepPrint();
    int _stopMoStepMax();
    int _stopMoStepCurrent();

    void _tlTimeSet(int);
    void _tlTimePrint();
    int _intMax();
    int _tlTimeCurrent();
    void _tlFramesPrint();
    int _tlFramesCurrent();
    void _recalcTLLength();
    void _tlLengthSet(int);
    void _tlLengthPrint();
    int _tlLengthCurrent();
    
    void _tlStartInSet(int);
    void _tlStartInPrint();
    int _tlStartInCurrent();
    
    void _tlPicMinuteSet(int);
    void _tlPicMinutePrint();
    int _tlPicMinuteCurrent();

    void _tlpModeTrigger(int);
    void _tlpModePrint();
    int _tlpModeCurrent();
    
    void _tlpTimeSet(int);
    void _tlpTimePrint();
    int _tlpTimeMin();
    int _tlpTimeMax();
    int _tlpTimeCurrent();
    
    void _tlpLengthSet(int);
    void _tlpLengthPrint();
    int _tlpLengthMin();
    int _tlpLengthMax();
    int _tlpLengthCurrent();
    
    void _tlpStartInSet(int);
    void _tlpStartInPrint();
    int _tlpStartInMax();
    int _tlpStartInCurrent();
    
    void _tlpLinearitySet(int);
    void _tlpLinearityPrint();
    int _tlpLinearityMax();
    int _tlpLinearityCurrent();
    
    void _tlpFramesPrint();
    void _tlpFirstMinPrint();
    void _tlpLastMinPrint();

    void _tlpCalcRealTime();
    
  private:
    bool _initializeEeprom();
    void _saveDataSlider();
    void _loadDataSlider();
    void _saveDataTL();
    void _loadDataTL();
    void _saveDataTLPro();
    void _loadDataTLPro();
    void _drawIcon(uint8_t icon);
    void _clearIcon(uint8_t icon);
    void _inputUpdated();
    void _tftMenuStringPrint(const uint8_t, const uint8_t);
    void _menuDraw();
    Adafruit_ST7735 &_tft;
    rotaryEncoder &_encoder;
    userInput &_userInput;
    int (menu::*_callbackReturn)();
    void (menu::*_callback)();
    void (menu::*_callbackEncoder)(int16_t);
    void _cb();
    int _cbReturn();
    void _cbSet(int16_t);
    void clearScreenLower();
    void _runInfo();
    volatile uint8_t _currentMode;
    volatile uint8_t _currentLevel;
    volatile uint8_t _currentMenu;
    int _currentPositionData;
    int16_t _oldEnc;
//    int16_t _encDiff;
    userInputDirection _userInputDirection = UI_STOP;
    volatile uint8_t _menuFlags = 0;
    char _string[9]; // temporary string array for printing on display
    uint8_t _batteryLevel = 0; // 0 - full, 1 - three fourths, 2 - two fourths, 3 - needs charging. 4 - not set
};


#endif


//  if (encoder.buttonPressed()) {
//    if (menuState[currentMenuState].enter != MS_NULL) {
//      currentMenuState = menuState[currentMenuState].enter;
//      Serial.print("Menu: ");
//      Serial.print(currentMenuState);
//      Serial.print(" ");
//      Serial.println(menuState[currentMenuState].text);
//      tftMenuPrint(menuState[currentMenuState].text);
//    } else {
//      Serial.println("No Submenu");
//      limitMax = menuState[currentMenuState].limitMinCB;
//      limitMin = menuState[currentMenuState].limitMaxCB;
//      infoCallback = menuState[currentMenuState].getCB;
//      currentPosition = menuState[currentMenuState].currentPositionCB;
//      callback = menuState[currentMenuState].setCB;
//      encoder.setLimits(limitMax(), limitMin());
//      encoder.setCurrentPosition(currentPosition());
//      encoderPrev = encoder.getCurrentPosition();
//      while(1) {
//        if (encoder.buttonPressed()) {
//          break;
//        }
//        if (encoder.getCurrentPosition() != encoderPrev) {
//          encoderPrev = encoder.getCurrentPosition();
//          callback(encoderPrev);
//          // Draw current setting on screen
//          tftMenuPrint(menuState[currentMenuState].text);
//          tftIntPrint(currentPosition());
//        }
//      }
//    }
//  }

/* menu navigation
  if (encoderPrev < encoder.getCurrentPosition()) {
    encoderPrev = encoder.getCurrentPosition();
    Serial.print("Menu: ");
    currentMenuState = menuState[currentMenuState].nextState;
    Serial.print(currentMenuState);
    Serial.print(" ");
    Serial.println(menuState[currentMenuState].text);
    tftMenuPrint(menuState[currentMenuState].text);
    tftIntPrint(28);
  } else if (encoderPrev > encoder.getCurrentPosition()) {
    encoderPrev = encoder.getCurrentPosition();
    Serial.print("Menu: ");
    currentMenuState = menuState[currentMenuState].prevState;
    Serial.print(currentMenuState);
    Serial.print(" ");
    Serial.println(menuState[currentMenuState].text);
    tftMenuPrint(menuState[currentMenuState].text);
    tftIntPrint(29);
  }
*/

 // mode menu =============
 // mode arrow left 7,33
// tft.drawBitmap(7, 33, 16, 25, modeArrowLeft);
// // mode arrow right 84, 33
// tft.drawBitmap(84, 33, 16, 25, modeArrowRight);
// // mode  24, 65
// tft.drawBitmap(25, 20, cameramode, 56, 41, ST7735_WHITE);
// drawSimpleIcon();
// // battery 3, 16, 37 * 10 pixels
// drawColorIcon(&tft, BATTERYThird);
// tft.drawBitmap(2, 3, 37, 10, BATTERY7);
// tft.drawBitmap(2, 3, 37, 10, BATTERY10);
// tft.drawBitmap(2, 3, 37, 10, BATTERY3);
// tft.drawBitmap(2, 3, 37, 10, BATTERY1);
// // menu chooser,  3, 107
// tft.drawBitmap(5, 89, menuArrowRight, 12, 18, ST7735_WHITE);
//// tft.drawBitmap(3, 3, repeat, 19, 17, ST7735_WHITE);
//// tft.drawBitmap(3, 3, alarmclock, 17, 17, ST7735_WHITE);
//  // menu text 20, 85; 20, 103; 20, 119
// tft.setFont(&MyriadProRegular10pt7b);
// tft.setTextSize(1);
// tft.setCursor(20, 85);
// tft.print("Menu1");
// tft.setCursor(20, 103);
// tft.print("Menu2");
// tft.setCursor(20, 119);
// tft.print("Menu3");



//  tft.fillScreen (ST7735_BLACK);
//  tft.fillRect (0, 0, 160, 16, ST7735_GREEN);
//  tft.fillRect (0, 24, 160, 16 ,ST7735_RED);

// delay(2000);
//
//  tft.fillScreen (ST7735_BLACK);
//  tft.fillRect (0, 0, 160, 16, ST7735_GREEN);
//  tft.fillRect (0, 24, 160, 16 ,ST7735_BLUE);
//  testdrawtext ("         111111111222123456789012345678901\nScreen is 21 x 20\ncharacters in size.\n", ST7735_WHITE);

//  tft.setCursor (1, 48);
//  tft.println ("\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz~ right up to ASCII 255\n\nThe quick brown fox  jumped over the lazy dog.\n\n");
//
//  tft.setTextColor(ST7735_RED);
//  tft.setCursor (1, 128-18);
//  tft.println ("Countdown:");

//  tft.setTextColor (ST7735_BLACK);
//  tft.setTextSize (2);
//  for (int i=6; i>=0; i--) {
//     tft.setCursor (2, 128-18);
//     tft.fillRect (0, 128-18, 160, 18, ST7735_RED);
//     tft.print (i);
//     tft.print (" seconds");
//     delay(1000);
//  }
