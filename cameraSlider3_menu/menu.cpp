#include "Arduino.h"
#include "menu.h"
#include <EEPROM.h>
// 0x8410 too dark. Trying three bits set for each 565 color
#define ST7735_GRAY 0xE71C

//uint16_t sliderLength = 750; // millimeters
//uint16_t sliderStep = 500; // minimal slider step in micrometers (1000 = 1 millimeter)
//uint16_t sliderStepTime = 15; // in milliseconds * 10
#define SLIDER_MAX_STEPS 600

// time lapse minimum time, minutes
#define TLMINTIME 12

extern void stopISR();
extern void startISR();

static int32_t forIterator; // used also as temporary variable
static int16_t tmpVar2; // another global temporary variable to save memory

volatile runTimeInfo_t runTimeInfo;
volatile runTimeConfig_t runTimeConfig;

enum bitFlags {
    FLAGredraw            = 0x1,
    FLAGdirectionUpdated  = 0x2,
    FLAGmodeIconDrawn     = 0x4,
    FLAGrepeatIconDrawn   = 0x8,
    FLAGinfoUpdated       = 0x10, // Slider speed changed or other mode info can be updated
    FLAGredrawBattery     = 0x20,
    FLAGdelayIconDrawn    = 0x40,
//    FLAG5 = 0x80,
};

typedef struct {
  const char * text;
  void (menu::*setCB)(int); // accepts integer between limits. 0 means unsettable (greyed out in menu)
  void (menu::*drawCB)(); // returns current status
  int (menu::*limitMinCB)(); // decimal minimal position for rotary encoder
  int (menu::*limitMaxCB)(); // decimal max positioon for rotary encoder
  int (menu::*currentPositionCB)(); // returns decimal position for rotary encoder
} menuItem_t;

/*
   contains onformation on actions in menu levels e.g.
    mode selection
    mode warning
    menu
    menu selected, etc

    0 means - do nothing on this action
*/
typedef struct {
  void (menu::*draw)();
  void (menu::*encoderChange)(int16_t);
  void (menu::*buttonPress)();
  void (menu::*longButtonPress)();
  void (menu::*directionDisabled)();
  void (menu::*directionEnabled)();
} menuLevel_t;

const menuLevel_t menuLevels[] = {
  // Draw                       Encoder                   Button                      LongButton                    DirOff                      DirOn
  {&menu::modeSelectDraw,    &menu::modeSelectEncoder,  &menu::modeSelectButton,    &menu::longButton,   0,                          0},
  {&menu::modeWarningDraw,   0,                         &menu::modeWarningButton,   &menu::longButton,   &menu::modeWarningDirOff,   &menu::modeWarningDirOn},
  {&menu::modeMenuDraw,      &menu::modeMenuEncoder,    &menu::modeMenuButton,      &menu::longButton,   0,                          &menu::modeMenuDirOn},
  {&menu::menuChangeDraw,    &menu::menuChangeEncoder,  &menu::menuChangeButton,    &menu::longButton,   0,                          0},
  {&menu::waitRunDraw,       0,                         0,                          0,                   &menu::waitRunDirOff,       0},
  {&menu::runDraw,           &menu::runEncoder,         &menu::runButton,           &menu::longButton,   &menu::runDirOff,           &menu::runDirOn},
  {&menu::viewConfigDraw,    &menu::viewConfigEncoder,  &menu::viewConfigButton,    0,                   &menu::viewConfigDirOff,    0},
  {&menu::finishedDraw,      0,                         &menu::finishedButton,      &menu::longButton,   &menu::finishedDirOff,      0},
  {&menu::finishedDirWDraw,  0,                         0,                          0,                   &menu::finishedDirWOff,     0},
};

typedef struct {
  uint8_t speed;
  uint8_t repeat;
} dataSlider_t;

typedef struct {
  uint16_t realTimeMinutes;
  uint32_t calculatedRealTime; // in 100ms units
  uint16_t lengthSeconds;
  uint16_t waitTime;
  uint16_t frames;
  uint16_t startInMinutes;
  uint16_t picPerMin;
  uint8_t motorStepCount;
  uint8_t repeat;
} dataTL_t;

typedef struct {
  uint8_t increase;
  uint16_t realTimeMinutes;
  uint16_t calculatedRealTime; // in 100ms units
  uint8_t lengthSeconds;
  uint16_t initialStep;
  int8_t waitTimeIncrease;
  uint16_t frames;
  uint16_t startInMinutes;
  uint8_t startPicPerMin;
  uint8_t endPicPerMin;
  uint8_t repeat;
  uint8_t motorStepCount;
  uint16_t decreasingInitialStep;
} dataTLPro_t;

typedef struct {
  uint8_t delaySeconds;
  uint8_t stepSize;
} dataStopMotion_t;

dataSlider_t dataSlider;
dataTL_t dataTL;
dataTLPro_t dataTLPro;
dataStopMotion_t dataStopMotion;

// icons
typedef struct {
  uint8_t iconLocation; // icon location table index
  const uint8_t *iconData;
  const uint16_t *iconPalette;
} icon_t;

typedef struct {
  uint8_t x;
  uint8_t y;
  uint8_t w;
  uint8_t h;
} iconLocation_t;

// Contains icon location and size
iconLocation_t iconLocations[] = {
  {120, 4, 37, 10}, // Battery
  {53, 25, 56, 41}, // Mode Icon
  {32, 35, 16, 25}, // Mode Arrow Left
  {112, 35, 16, 25}, // Mode Arrow Right
  {2, 3, 19, 17}, // Repeat Icon
  {29, 3, 17, 17}, // Alarm Icon
  {4, 91, 11, 19}, // Menu icon / Menu icon selected
};

enum iconName {
  BATTERYFULL,
  BATTERYMOST,
  BATTERYTHIRD,
  BATTERYEMPTY,
  MODESLIDER,
  MODETL,
  MODEPTL,
  MODESTOPGO,
  MODEARROWRIGHT,
  MODEARROWLEFT,
  ARROWWORKRIGHT,
  ARROWWORKLEFT,
  ARROWSTOPRIGHT,
  ARROWSTOPLEFT,
  ICONREPEAT,
  ICONDELAY,
  ICONMENU,
  ICONMENUMOD,
} iconName_t;


//  uint8_t iconLocation;
//  char *imageLoc;
//  char *somecolor;
//  iconType type;
icon_t icons[] = {
  {0, batteryFull, batteryColorPalette},
  {0, batteryMost, batteryColorPalette},
  {0, batteryThird, batteryColorPalette},
  {0, batteryEmpty, batteryColorPalette},
  {1, modeSlider, modeColorPalette},
  {1, modeTimeLapse, modeColorPalette},
  {1, modeTimeLapsePro, modeColorPalette},
  {1, modeStopMotion, modeColorPalette},
  {2, menuArrowYellowLeft, arrowColorPalette},
  {3, menuArrowYellowRight, arrowColorPalette},
  {2, menuArrowRedLeft, arrowColorPalette},
  {3, menuArrowRedRight, arrowColorPalette},
  {2, menuArrowGreenLeft, arrowColorPalette},
  {3, menuArrowGreenRight, arrowColorPalette},
  {4, iconRepeat, iconColorPalette},
  {5, iconAlarm, iconColorPalette},
  {6, iconArrowRight, iconColorPalette},
  {6, iconArrowSelected, iconArrowSelectedColorPalette},
};

const char * const menuTLStateName[] = {
  "Real Time",
  "Video Length",
  "Pic/Min",
  "Frames",
  "Start In",
//  "Repeat",
//  "Preset",
};

const menuItem_t menuTL[] = {
  //text,              setCB,              getCB,                   limitMinCB,          limitMaxCB,         currentPosition
  {menuTLStateName[0], &menu::_tlTimeSet,  &menu::_tlTimePrint,        &menu::_limitMinZero,   &menu::_intMax, &menu::_tlTimeCurrent},
  {menuTLStateName[1], &menu::_tlLengthSet,  &menu::_tlLengthPrint,    &menu::_limitMinZero,   &menu::_intMax, &menu::_tlLengthCurrent},
  {menuTLStateName[2], 0, /*&menu::_tlPicMinuteSet,*/ &menu::_tlPicMinutePrint,  &menu::_limitMinZero,&menu::_intMax, &menu::_tlPicMinuteCurrent},
  {menuTLStateName[3], 0, /*&menu::_tlFramesSet,*/  &menu::_tlFramesPrint,    &menu::_limitMinZero,   &menu::_intMax, &menu::_tlFramesCurrent},
  {menuTLStateName[4], &menu::_tlStartInSet, &menu::_tlStartInPrint,   &menu::_limitMinZero,   &menu::_intMax, &menu::_tlStartInCurrent},
//  {menuTLStateName[5], &menu::_tlRepeatSet,  &menu::_tlRepeatPrint,    &menu::limitMinCB,   &menu::limitMaxCB, &menu::currentPositionCB},
//  {menuTLStateName[6], &menu::_tlPresetSet,  &menu::_tlPresetPrint,  &menu::_limitOne,   &menu::_presetsMax, &menu::_tlCurrentPreset},
};


const char * const menuTLProStateName[] = {
  "Type",
  "Real Time",
  "Video Length",
//  "Linearity",
  "Start In",
  "Frames",
  "A/Frames/M",
  "B/Frames/M",
//  "Repeat",
//  "Preset",
};

const menuItem_t menuTLPro[] = {
  //text,                 setCB,              getCB,                      limitMinCB,          limitMaxCB,         currentPosition
  {menuTLProStateName[0], &menu::_tlpModeTrigger,&menu::_tlpModePrint,       &menu::_limitMinZero,   &menu::_intMax,        &menu::_tlpModeCurrent},
  {menuTLProStateName[1], &menu::_tlpTimeSet,   &menu::_tlpTimePrint,         &menu::_tlpTimeMin,   &menu::_tlpTimeMax, &menu::_tlpTimeCurrent},
  {menuTLProStateName[2], &menu::_tlpLengthSet, &menu::_tlpLengthPrint,       &menu::_tlpLengthMin,   &menu::_tlpLengthMax, &menu::_tlpLengthCurrent},
  {menuTLProStateName[3], &menu::_tlpStartInSet,&menu::_tlpStartInPrint,      &menu::_limitMinZero,   &menu::_tlpStartInMax, &menu::_tlpStartInCurrent},
//  {menuTLProStateName[4], &menu::_tlpLinearitySet,&menu::_tlpLinearityPrint,  &menu::_limitMinZero,   &menu::_tlpLinearityMax, &menu::_tlpLinearityCurrent},
  {menuTLProStateName[4], 0,                    &menu::_tlpFramesPrint,       0,           0,                  0},
  {menuTLProStateName[5], 0,                    &menu::_tlpFirstMinPrint,     0,           0,                  0},
  {menuTLProStateName[6], 0,                    &menu::_tlpLastMinPrint,      0,           0,                  0},
//  {menuTLProStateName[7], &menu::oneFuncCB,  &menu::twoInfoCB,        &menu::limitMinCB,   &menu::limitMaxCB, &menu::currentPositionCB},
//  {menuTLProStateName[8], &menu::_tlpPresetSet,  &menu::_tlpPresetPrint,     &menu::_limitOne,   &menu::_presetsMax, &menu::_tlpCurrentPreset},
};

const char * const menuStopMotionStateName[] = {
  "Delay",
  "Step",
};

const menuItem_t menuStopMotion[] = {
  //text,                       setCB,              getCB,                   limitMinCB,          limitMaxCB,         currentPosition
  {menuStopMotionStateName[0], &menu::_stopMoDelaySet,  &menu::_stopMoDelayPrint,        &menu::_limitMinOne,   &menu::_stopMoDelayMax, &menu::_stopMoDelayCurrent},
  {menuStopMotionStateName[1], &menu::_stopMoStepSet,  &menu::_stopMoStepPrint,        &menu::_limitMinOne,   &menu::_stopMoStepMax, &menu::_stopMoStepCurrent},
};

const menuItem_t * currentModeMenu;

typedef struct {
  uint8_t x;
  uint8_t y;
  const GFXfont * font;
  char const * text;
} modeChooseText_t;

const modeChooseText_t modeChooseText[][3] = {
  {
    {55, 103, &MyriadProRegular10pt7b, "Slider"},
//    {0, 103, "Smooth camera movement"},
//    {13, 115, "some other text"},
  },
  {
    {32, 103, &MyriadProRegular10pt7b, "Time Lapse"},
//    {0, 103, "Smooth camera movement"},
//    {13, 115, "some other text"},
  },
  {
    {28, 90, &MyriadProRegular10pt7b, "Progressive"},
    {30, 115, &MyriadProRegular10pt7b, "Time Lapse"},
//    {13, 115, "some other text"},
  },
  {
    {25, 103, &MyriadProRegular10pt7b, "Stop Motion"},
//    {0, 103, "Smooth camera movement"},
//    {13, 115, "some other text"},
  },
};

const modeChooseText_t modeWarning[] = {
  {48, 90, &MyriadProRegular7pt7b, "Warning"},
  {8, 103, &MyriadProRegular7pt7b, "Please move camera to"},
  {19, 115, &MyriadProRegular7pt7b, "the end of the slider"},
};

const modeChooseText_t finishedWarning[] = {
  {48, 90, &MyriadProRegular7pt7b, "Warning"},
  {0, 103, &MyriadProRegular7pt7b, "Please disable movement"},
};

const char startInText[] = "Start in";

typedef struct {
  const menuItem_t *menu;
  const uint8_t amount;
} menuItems_t;

// Number of items in each menu
uint8_t menuItemAmount[] = {0, 5, 7, 2};

const menuItems_t menus[] = {
  {menuTL, menuItemAmount[0]},
  {menuTL, menuItemAmount[1]},
  {menuTLPro, menuItemAmount[2]},
  {menuStopMotion, menuItemAmount[3]},
};

typedef struct {
  uint8_t x;
  uint8_t y;
  uint8_t datax;
  uint8_t datay;
} menuInfoLocations_t;


typedef struct {
  const char * text;
  void (menu::*getCB)();
} runInfo_t;

const char * const runInfoText[] = {
  // time lapse & time lapse pro
  "Pictures",
  "Next Pic",
  "Time Left",
  // stop motion
  "Pictures",
  "Length",
};

runInfo_t runInfo[][3] = {
  // nothing for slider
  { {0,0}, {0,0}, {0,0} },
  {
    {runInfoText[0], &menu::picsLeftAndTotal },
    {runInfoText[1], &menu::nextPic },
    {runInfoText[2], &menu::timeLeft },
  },
  {
    {runInfoText[0], &menu::picsLeftAndTotal },
    {runInfoText[1], &menu::nextPic },
    {runInfoText[2], &menu::timeLeft },
  },
  {
    {runInfoText[3], &menu::picsTaken },
    {runInfoText[4], &menu::videoLength },
    {0, 0 },
  }
};


// data used in recursive calcTLPro() routine
typedef struct {
    uint16_t stepsDone;
    uint16_t totalFrames;
    uint8_t calcMotorStepMultiplier;
    uint32_t totalTime; // tenths of a second
    uint16_t currentFramePause;
} calcTLProVars_t;
calcTLProVars_t calcTLProVars;


menu::menu(Adafruit_ST7735 &tft, rotaryEncoder &encoder, userInput &userInput) : _tft(tft), _encoder(encoder), _userInput(userInput) {
  _currentPositionData = 22;
  _currentMenu = 0;
}

void menu::init() {
  _tft.initR(INITR_BLACKTAB);
  _tft.setRotation(1); // 0 - Portrait, 1 - Landscape
  _tft.fillScreen(ST7735_BLACK);
  _tft.setFont(&MyriadProRegular7pt7b);
  _tft.setTextWrap(true);
  _tft.setTextSize(1);
  _tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  _currentMode = 0;
  _currentLevel = 5;
  // set encoder for slider mode
  _encoder.setLimits(0, 100);
  if (!_initializeEeprom()) {
    _loadDataSlider();
    _loadDataTL();
    _loadDataTLPro();
  }
  _encoder.setCurrentPosition(dataSlider.speed);
  _oldEnc = dataSlider.speed;
  _encoder.setStep(1);
  _menuFlags |= FLAGinfoUpdated;
  _menuFlags |= FLAGredraw;
  _batteryLevel = 4;
}

void menu::infoUpdated() {
  noInterrupts();
  _menuFlags |= FLAGredraw;
  _menuFlags |= FLAGinfoUpdated;
  interrupts();
}

void menu::sliderFinished() {
  _currentLevel = 7;
  runTimeInfo.nextFrameTime = 0;
  infoUpdated();
}

void menu::_drawIcon(uint8_t icon) {
  _tft.drawBitmap(iconLocations[icons[icon].iconLocation].x,
                  iconLocations[icons[icon].iconLocation].y,
                  iconLocations[icons[icon].iconLocation].w,
                  iconLocations[icons[icon].iconLocation].h,
                  icons[icon].iconData, icons[icon].iconPalette);
}

void menu::_clearIcon(uint8_t icon) {
  _tft.fillRect(iconLocations[icons[icon].iconLocation].x,
                iconLocations[icons[icon].iconLocation].y,
                iconLocations[icons[icon].iconLocation].w,
                iconLocations[icons[icon].iconLocation].h,
                ST7735_BLACK);
}

void menu::updateBatteryStatus(uint16_t value) {
  // voltage = value / 100
  if (value >= 1520) {
    if (_batteryLevel != 0) {
      _batteryLevel = 0;
      _menuFlags |= FLAGredrawBattery;
    }
  } else if (value >= 1440) {
    if (_batteryLevel != 1) {
      _batteryLevel = 1;
      _menuFlags |= FLAGredrawBattery;
    }
  } else if (value >= 1360) {
    if (_batteryLevel != 2) {
      _batteryLevel = 2;
      _menuFlags |= FLAGredrawBattery;
    }
  } else {
    if (_batteryLevel != 3) {
      _batteryLevel = 3;
      _menuFlags |= FLAGredrawBattery;
    }
  }
}

void menu::startInFinished() {
  _currentLevel = 5;
}

int menu::getCurrentPosition() {
  _callbackReturn =  menus[_currentMode].menu[_currentLevel].currentPositionCB;

  menu * self = static_cast<menu*>(this);
  return (self->*_callbackReturn)();
}

void menu::draw() {
  if ((_menuFlags & FLAGredraw) == FLAGredraw) {
    noInterrupts();
    _menuFlags &= ~FLAGredraw;
    interrupts();
    if (_currentMode == 0 && (_currentLevel != 0 && _currentLevel != 5)) {
      _currentMode = 1;
      _currentLevel = 8;
    }
//    Serial.print(F("Drawing level "));
//    Serial.print(_currentLevel);
//    Serial.print(F(" Drawing mode "));
//    Serial.print(_currentMode);
//    Serial.print(F(" Drawing menu "));
//    Serial.println(_currentMenu);
    _callback = menuLevels[_currentLevel].draw;
    _cb();
  }
  if ((_menuFlags & FLAGredrawBattery) == FLAGredrawBattery) {
    _menuFlags &= ~FLAGredrawBattery;
    if (_batteryLevel <= 3) { // Draw only valid battery level
      _drawIcon(BATTERYFULL + _batteryLevel);
    }
  }
}

int freeRam2() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void menu::_cb() {
  if (_callback != NULL) {
    menu * self = static_cast<menu*>(this);
    (self->*_callback)();
  }
}

int menu::_cbReturn() {
  if (_callbackReturn != NULL) {
    menu * self = static_cast<menu*>(this);
    return (self->*_callbackReturn)();
  }
  return 0;
}

void menu::_cbSet(int16_t value) {
  if (_callbackEncoder != NULL) {
    menu * self = static_cast<menu*>(this);
    (self->*_callbackEncoder)(value);
  }
}


// All menu actions are initiated from here
void menu::inputUpdate() {
  _userInput.readSensors();
  if (_userInput.getDirection() != _userInputDirection) {
    _userInputDirection = _userInput.getDirection();
    Serial.print(F("Input direction changed: " ));
    Serial.print(_userInput.getDirection());
    Serial.print(F(" menuLevel: "));
    Serial.println(_currentLevel);
    if (_userInputDirection != UI_STOP) {
      _callback = menuLevels[_currentLevel].directionEnabled;
    } else {
      _callback = menuLevels[_currentLevel].directionDisabled;
    }
    _cb();
    _menuFlags |= FLAGdirectionUpdated;
    _inputUpdated();
  }

  if (_encoder.buttonPressedReleased()) {
    Serial.print(F("Button pressed. CurrentMode: "));
    Serial.print(_currentMode);
    Serial.print(F(" currentLevel: "));
    Serial.println(_currentLevel);
    // disallow pressing button only in time lapse, TLPro, SM mode when driving
    if (_currentLevel != 5 && _userInputDirection == UI_STOP) {
      _callback = menuLevels[_currentLevel].buttonPress;
    } else if (_currentMode == 0) { // always allow setting repeat
      _callback = menuLevels[_currentLevel].buttonPress;
    }else {
      _callback = NULL;
    }
    _cb();
    if (_callback != NULL) {
      _inputUpdated();
    }
  }

  if (_encoder.buttonLongPressed()) {
    Serial.print(F("Button longpressed. Direction: "));
    Serial.print(userInputDirectionString[_userInputDirection]);
    Serial.print(F(" currentLevel: "));
    Serial.println(_currentLevel);
    // Prevent slider from going to mode select when running
    if (_userInputDirection == UI_STOP) {
      _callback = menuLevels[_currentLevel].longButtonPress;
    } else {
      _callback = NULL;
    }
    _cb();
    if (_callback != NULL) {
      _inputUpdated();
    }
  }

  if (_encoder.getCurrentPosition() != _oldEnc) {
    Serial.print(F("Encoder updated. Old "));
    Serial.print(_oldEnc);
    Serial.print(" new: ");
    Serial.println(_encoder.getCurrentPosition());
    _callbackEncoder = menuLevels[_currentLevel].encoderChange;
    if (_callbackEncoder != NULL) {
      menu * self = static_cast<menu*>(this);
      // calculate flag is set inside callback function
      (self->*_callbackEncoder)(_encoder.getCurrentPosition());
      _inputUpdated();
    }
//    _encDiff = _encoder.getCurrentPosition() - _oldEnc;
    _oldEnc = _encoder.getCurrentPosition();
  }
}

// redraw screen
void menu::_inputUpdated() {
  noInterrupts();
  _menuFlags |= FLAGredraw;
  interrupts();
}

/*
   Draw functions
*/
void menu::clearScreenLower() {
  _tft.fillRect(0, 65, 160, 128, ST7735_BLACK);
}

void menu::modeSelectDraw() {
  clearScreenLower();
  if (((_menuFlags & FLAGrepeatIconDrawn) == FLAGrepeatIconDrawn) && (_currentMode != 0)) {
    _clearIcon(ICONREPEAT);
    _menuFlags &= ~FLAGrepeatIconDrawn;
  }
  
  if ((_menuFlags & FLAGmodeIconDrawn) == FLAGmodeIconDrawn) {
    _menuFlags &= ~FLAGmodeIconDrawn;
    _clearIcon(MODESLIDER);
    _tft.setTextSize(1);
  }
  for (forIterator = 0; forIterator < 3; forIterator++) {
    _tft.setCursor(modeChooseText[_currentMode][forIterator].x, modeChooseText[_currentMode][forIterator].y);
    _tft.setFont(modeChooseText[_currentMode][forIterator].font);
    _tft.print(modeChooseText[_currentMode][forIterator].text);
  }
  _tft.setFont(&MyriadProRegular7pt7b);
  _drawIcon(MODESLIDER + _currentMode);
  if (!((_menuFlags & FLAGdirectionUpdated) == FLAGdirectionUpdated)) {
    _drawIcon(MODEARROWLEFT);
    _drawIcon(MODEARROWRIGHT);
    _menuFlags &= ~FLAGdirectionUpdated;
  }
  //  Serial.print("Memory free: ");
  //  Serial.println(freeRam2());
}

void menu::modeWarningDraw() {
  _clearIcon(MODEARROWLEFT);
  _clearIcon(MODEARROWRIGHT);
  _menuFlags &= ~FLAGdirectionUpdated;
  _menuFlags |= FLAGmodeIconDrawn;
  if (_currentMode == 1) {
    currentModeMenu = menuTL;
  } else if (_currentMode == 2) {
    currentModeMenu = menuTLPro;
  } else if (_currentMode == 3) {
    currentModeMenu = menuStopMotion;
  }
  clearScreenLower();
  for (forIterator = 0; forIterator < 3; forIterator++) {
    _tft.setCursor(modeWarning[forIterator].x, modeWarning[forIterator].y);
    _tft.print(modeWarning[forIterator].text);
  }
}

void menu::modeMenuDraw() {
  clearScreenLower();
  _clearIcon(MODEARROWLEFT);
  _clearIcon(MODEARROWRIGHT);
  _menuFlags &= ~FLAGdirectionUpdated;
  _drawIcon(ICONMENU);
  _menuDraw();
}

void menu::menuChangeDraw() {
  clearScreenLower();
  _drawIcon(ICONMENUMOD);
  _callbackReturn = currentModeMenu[_currentMenu].limitMinCB;
  forIterator = _cbReturn();
  _callbackReturn = currentModeMenu[_currentMenu].limitMaxCB;
  _encoder.setLimits(forIterator, _cbReturn());
  _callbackReturn = currentModeMenu[_currentMenu].currentPositionCB;
  _encoder.setCurrentPosition(_cbReturn());
  _oldEnc = _encoder.getCurrentPosition();
  _menuDraw();
}

void menu::_menuDraw() {
  if (!((_menuFlags & FLAGdelayIconDrawn) == FLAGdelayIconDrawn) && 
  ((dataTL.startInMinutes && _currentMode == 1) || (dataTLPro.startInMinutes && _currentMode == 2))) {
    _drawIcon(ICONDELAY);
    _menuFlags |= FLAGdelayIconDrawn;
  } else if (((_menuFlags & FLAGdelayIconDrawn) == FLAGdelayIconDrawn) && 
  !((dataTL.startInMinutes && _currentMode == 1) || (dataTLPro.startInMinutes && _currentMode == 2))) {
    _clearIcon(ICONDELAY);
    _menuFlags &= ~FLAGdelayIconDrawn;
  }
  
  int8_t tmpVar = _currentMenu - 1;
  if (menuItemAmount[_currentMode] > 1) {
    _tft.setCursor(20, 90);
    if (tmpVar < 0) {
      tmpVar = menus[_currentMode].amount - 1;
    }
    if (_currentLevel == 6 || menus[_currentMode].menu[tmpVar].setCB == NULL) {
      _tft.setTextColor(ST7735_GRAY, ST7735_BLACK);
    } else {
      _tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    }
//    Serial.print(F("First: "));
//    Serial.print(tmpVar);
//    Serial.print(F(" second: "));
    _tft.print(menus[_currentMode].menu[tmpVar].text);
    _tft.setCursor(100, 90);
    _callback = menus[_currentMode].menu[tmpVar].drawCB;
    _cb();
  }
  tmpVar = (tmpVar + 1) % (menus[_currentMode].amount);
//  Serial.print(tmpVar);
  if (_currentLevel == 6 || menus[_currentMode].menu[tmpVar].setCB == NULL) {
    _tft.setTextColor(ST7735_GRAY, ST7735_BLACK);
  } else {
    _tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  }
  _tft.setCursor(20, 105);
  _tft.print(menus[_currentMode].menu[tmpVar].text);
  _tft.setCursor(100, 105);
  _callback = menus[_currentMode].menu[tmpVar].drawCB;
  _cb();
  if (menuItemAmount[_currentMode] > 2) {
    tmpVar = (_currentMenu + 1) % (menus[_currentMode].amount);
    if (_currentLevel == 6 || menus[_currentMode].menu[tmpVar].setCB == NULL) {
      _tft.setTextColor(ST7735_GRAY, ST7735_BLACK);
    } else {
      _tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    }
//    Serial.print(F(" third: "));
//    Serial.println(tmpVar);
    _tft.setCursor(20, 120);
    _tft.print(menus[_currentMode].menu[tmpVar].text);
    _callback = menus[_currentMode].menu[tmpVar].drawCB;
    _tft.setCursor(100, 120);
    _cb();
  }
//  if (_currentLevel == 6) {
    _tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
//  }
}

void menu::waitRunDraw() {
  clearScreenLower();
  // draw direction icon according to direction button
  if ((_menuFlags & FLAGdirectionUpdated) == FLAGdirectionUpdated) {
    _menuFlags &= ~FLAGdirectionUpdated;
    if (_userInput.getDirection() == UI_LEFT) {
      _clearIcon(ARROWWORKRIGHT);
      _drawIcon(ARROWWORKLEFT);
    } else if (_userInput.getDirection() == UI_RIGHT) {
      _clearIcon(ARROWWORKLEFT);
      _drawIcon(ARROWWORKRIGHT);
    } else if (_userInput.getDirection() == UI_STOP) {
      _clearIcon(ARROWWORKRIGHT);
      _clearIcon(ARROWWORKLEFT);
    }
  }

  strncpy(_string, startInText, 9); // Normunds
  _tft.setFont(&MyriadProRegular10pt7b);
  _tft.setCursor(45, 90);
  _tft.print(_string);
  snprintf(_string, 9, " %lum:%lus", runTimeInfo.startTimeLeft / 100 / 60, runTimeInfo.startTimeLeft / 100 % 60);
  _tft.setCursor(45, 115);
  _tft.print(_string);
  _tft.setFont(&MyriadProRegular7pt7b);
}

void menu::runDraw() {
  if (!((_menuFlags & FLAGrepeatIconDrawn) == FLAGrepeatIconDrawn) && dataSlider.repeat) {
    _drawIcon(ICONREPEAT);
    _menuFlags |= FLAGrepeatIconDrawn;
  } else if (((_menuFlags & FLAGrepeatIconDrawn) == FLAGrepeatIconDrawn) && (!dataSlider.repeat || !(_currentMode == 0))) {
    _clearIcon(ICONREPEAT);
    _menuFlags &= ~FLAGrepeatIconDrawn;
  }
  
  if (!((_menuFlags & FLAGmodeIconDrawn) == FLAGmodeIconDrawn)) {
    _drawIcon(MODESLIDER);
    _menuFlags |= FLAGmodeIconDrawn;
    _encoder.setStep(5);
  }
  
  // draw direction icon according to direction button
  if ((_menuFlags & FLAGdirectionUpdated) == FLAGdirectionUpdated) {
    _menuFlags &= ~FLAGdirectionUpdated;
    if (_userInput.getDirection() == UI_LEFT) {
      _clearIcon(ARROWWORKRIGHT);
      _drawIcon(ARROWWORKLEFT);
    } else if (_userInput.getDirection() == UI_RIGHT) {
      _clearIcon(ARROWWORKLEFT);
      _drawIcon(ARROWWORKRIGHT);
    } else if (_userInput.getDirection() == UI_STOP) {
      _clearIcon(ARROWWORKRIGHT);
      _clearIcon(ARROWWORKLEFT);
    }
  }

  // For slider mode, draw current speed
  if ((_menuFlags & FLAGinfoUpdated) == FLAGinfoUpdated) {
    _menuFlags &= ~FLAGinfoUpdated;
    clearScreenLower();
    if (_currentMode == 0) {
      _tft.setFont(&MyriadPro_Regular28pt7bNum);
      if (dataSlider.speed > 9) {
        _tft.setCursor(50, 115);
      } else {
        _tft.setCursor(65, 115);
      }
      snprintf(_string, 9, "%d", dataSlider.speed);
      _tft.print(_string);
      _tft.setFont(&MyriadProRegular7pt7b);
    } else {
      _tft.setTextSize(1);
      clearScreenLower();
      _runInfo();
    }
  }
}

// Draw runtime information for each mode
void menu::_runInfo() {
  _tft.setCursor(20, 90);
  _tft.print(runInfo[_currentMode][0].text);
  _tft.setCursor(80, 90);
  _callback = runInfo[_currentMode][0].getCB;
  _cb();
  _tft.setCursor(20, 105);
  _tft.print(runInfo[_currentMode][1].text);
  _tft.setCursor(80, 105);
  _callback = runInfo[_currentMode][1].getCB;
  _cb();
  _tft.setCursor(20, 120);
  _tft.print(runInfo[_currentMode][2].text);
  _tft.setCursor(80, 120);
  _callback = runInfo[_currentMode][2].getCB;
  _cb();
}

void menu::viewConfigDraw() {
  clearScreenLower();
  _menuDraw();
}

void menu::finishedDraw() {
  _drawIcon(ARROWSTOPLEFT);
  _drawIcon(ARROWSTOPRIGHT);
  _tft.setTextSize(1);
  clearScreenLower();
  _runInfo();
}

void menu::finishedDirWDraw() {
  clearScreenLower();
  for (forIterator = 0; forIterator < 2; forIterator++) {
    _tft.setCursor(finishedWarning[forIterator].x, finishedWarning[forIterator].y);
    _tft.print(finishedWarning[forIterator].text);
  }
}


/*
   Encoder input functions
*/
void menu::modeSelectEncoder(int16_t value) {
  if (value < _oldEnc) {
    _currentMode = _currentMode - 1;
    _currentMode = (_currentMode % 4);
  } else if (value > _oldEnc) {
    _currentMode = (_currentMode + 1) % 4;
  }
}

void menu::modeMenuEncoder(int16_t value) {
  // move to next/prev _currentMenu
  if (value < _oldEnc) {
    _currentMenu = (_currentMenu - 1);
    if (_currentMenu > menus[_currentMode].amount - 1) {
      _currentMenu = menus[_currentMode].amount - 1;
    }
//    Serial.print("Changing currentMenu to ");
//    Serial.println(_currentMenu);
  } else if (value > _oldEnc) {
    _currentMenu = ((_currentMenu + 1) % menus[_currentMode].amount);
//    Serial.print("Changing currentMenu to ");
//    Serial.println(_currentMenu);
  }
}

void menu::menuChangeEncoder(int16_t value) {
  // feed changed value to menu setting callback
  _callbackEncoder = currentModeMenu[_currentMenu].setCB;
  _cbSet(value);
}

void menu::runEncoder(int16_t value) {
  // On slider: change speed
  if (_currentMode == 0) {
    runTimeConfig.speed = dataSlider.speed = value;
    _menuFlags |= FLAGinfoUpdated;
    if (dataSlider.speed >= 99) {
      dataSlider.speed = 99;
    }
    runTimeConfig.speed = map(dataSlider.speed, 0, 99, 0, 255);
  }
}

void menu::viewConfigEncoder(int16_t value) {
  (void) value;
  // do nothing. Draw() will take care of menu navigation
}


/*
   Select button functions
*/
void menu::modeSelectButton() {
  _currentMenu = 0;
  if (_currentMode == 0) {
    runTimeConfig.sliderMode = 0;
    runTimeConfig.frames = 0;
    runTimeConfig.motorStep = 0;
    runTimeConfig.timeBetweenFrames = 0;
    runTimeConfig.motorStepMultiplier = 0;
    _clearIcon(MODEARROWLEFT);
    _clearIcon(MODEARROWRIGHT);
    _currentLevel = 5; // for slider
    _encoder.setLimits(0, 100); // TODO remember speed from last time
    _encoder.setCurrentPosition(0);
    _oldEnc = 0;
    _encoder.setStep(1);
    _menuFlags |= FLAGinfoUpdated;
  } else {
    _menuFlags |= FLAGmodeIconDrawn;
    _currentLevel = 1; // for time lapse, tlpro, stop mo
  }
}

void menu::modeWarningButton() {
  _currentLevel = 2;
}

void menu::modeMenuButton() {
  // enter menu editing mode, if menu item is editable
  if (menus[_currentMode].menu[_currentMenu].setCB != NULL) {
    _currentLevel = 3;
  }
}

void menu::menuChangeButton() {
  // go back to menu overview mode
  _currentLevel = 2;
  _encoder.setLimits(-100, 100);
  _encoder.setCurrentPosition(0);
  _oldEnc = 0;
  _encoder.setStep(1);
  if (_currentMode == 1) {
    _saveDataTL();
  } else if (_currentMode == 2) {
    _saveDataTLPro();
  }
}

void menu::runButton() {
  Serial.println(F("runButton()"));
  if (_currentMode == 0) {
    if (dataSlider.repeat == 0) {
      runTimeInfo.repeat = dataSlider.repeat = 10;
    } else {
      runTimeInfo.repeat = dataSlider.repeat = 0;
    }
    Serial.println(dataSlider.repeat);
    _inputUpdated();
  } else {
    _currentLevel = 3;
  }
}

void menu::viewConfigButton() {
  Serial.println(F("viewConfigButton()"));
  _currentLevel = 5;
}

void menu::finishedButton() {
  Serial.println(F("finishedButton()"));
  _currentLevel = 2;
}


void menu::longButton() {
  Serial.println(F("longButton()"));
  _currentLevel = 0;
  _encoder.setLimits(-100, 100);
  _encoder.setCurrentPosition(0);
  _oldEnc = 0;
  _encoder.setStep(1);
}


/*
   DirOff functions
*/
void menu::waitRunDirOff() {
  Serial.println(F("waitRunDirOff()"));
  _currentLevel = 2;
  stopISR();
}

void menu::runDirOff() {
  Serial.println(F("runDirOff()"));
  if (_currentMode != 0) {
    _currentLevel = 2;
  }
  stopISR();
  if (_currentMode == 0) {
    _saveDataSlider();
  }
}

void menu::runDirOn() {
  if (_currentMode == 0) {
    if (dataSlider.repeat) {
      dataSlider.repeat = 10;
    }
    startISR();
  }
}

void menu::viewConfigDirOff() {
  Serial.println(F("viewConfigDirOff()"));
  _currentLevel = 2;
}

void menu::finishedDirOff() {
  Serial.println(F("finishedDirOff()"));
  _currentLevel = 2;
}

void menu::finishedDirWOff() {
  Serial.println(F("finishedDirWOff()"));
  _currentLevel = 2;
}


/*
   DirOn Functions
*/
void menu::modeWarningDirOn() {
  Serial.println(F("modeWarningDirOn()"));
  runTimeConfig.sliderMode = 0; // enable slider mode
  runTimeConfig.speed = 180;
  startISR(); // enable ISR
}

void menu::modeWarningDirOff() {
  Serial.println(F("modeWarningDirOff()"));
  stopISR(); // disable ISR
}

void menu::modeMenuDirOn() {
  Serial.println(F("modeMenuDirOn()"));
  _menuFlags |= FLAGinfoUpdated;
  _currentLevel = 5;
  runTimeConfig.sliderMode = _currentMode;
  // set required frames count
  runTimeConfig.picReleaseTime = 2;
  runTimeConfig.waitBeforePic = 3;
  if (_currentMode == 1) {
    runTimeConfig.frames = dataTL.frames;
    // set wait time between frames
    runTimeConfig.timeBetweenFrames = dataTL.waitTime;
    // set wait increase / decrease amount
    runTimeConfig.motorWaitIncreaseStep = 0;
    // how long are we running motor
    runTimeConfig.motorStep = ((dataTL.motorStepCount * 1.33) + 0.5);
    runTimeConfig.waitBeforePic = 5;
    runTimeConfig.startTime = millis();
    runTimeConfig.endTime = runTimeConfig.startTime + (dataTL.realTimeMinutes * 60UL) * 1000UL;

    // calculate real time
    dataTL.calculatedRealTime = 0;
    // first picture consists of waiting for motor stop (0) + wait for pic + take pic time
    dataTL.calculatedRealTime += runTimeConfig.waitBeforePic + runTimeConfig.picReleaseTime;
    dataTL.calculatedRealTime += (uint16_t) (dataTL.frames - 1) * dataTL.waitTime;
    runTimeInfo.endTime = dataTL.calculatedRealTime * 10UL;
    runTimeConfig.motorStepMultiplier = 1;

    Serial.print("Calculated real time(sec * 10): ");
    Serial.print(dataTL.calculatedRealTime);
    Serial.print(" Realtime: ");
    Serial.println(dataTL.realTimeMinutes * 60);
    if (dataTL.startInMinutes) {
      runTimeInfo.startTimeLeft = dataTL.startInMinutes * 60UL * 100UL;
      _currentLevel = 4;
    }
    startISR();
  } else if (_currentMode == 2) {
    runTimeConfig.frames = dataTLPro.frames;
    // set initial wait time
    runTimeConfig.timeBetweenFrames = dataTLPro.initialStep;
    // set wait increase / decrease amount
    runTimeConfig.motorWaitIncreaseStep = dataTLPro.waitTimeIncrease;
    // set motor enable duration
    runTimeConfig.motorStep = dataTLPro.motorStepCount; // Normunds
    runTimeConfig.startTime = millis();
    // TODO add 999 so we do not overflow at the end?
    runTimeConfig.endTime = runTimeConfig.startTime + dataTLPro.calculatedRealTime * 100UL;
    runTimeInfo.endTime = dataTLPro.calculatedRealTime * 10UL;
    Serial.print("Calculated step for multiplier ");
    Serial.print(calcTLProVars.calcMotorStepMultiplier);
    Serial.print(" : ");
    Serial.print(((calcTLProVars.calcMotorStepMultiplier * 1.33) + 0.5) / 10);
    runTimeConfig.motorStepMultiplier = ((calcTLProVars.calcMotorStepMultiplier * 1.33) + 0.5) / 10;
    Serial.print(" Initial endTime: ");
    Serial.println(runTimeInfo.endTime);
    if (dataTLPro.startInMinutes) {
      runTimeInfo.startTimeLeft = dataTLPro.startInMinutes * 60UL * 100UL;
      _currentLevel = 4;
    }
    startISR();
  } else if (_currentMode == 3) {
    runTimeInfo.picsTaken = 0;
    runTimeConfig.frames = 1;
    runTimeConfig.motorStep = 1;
    runTimeConfig.timeBetweenFrames = dataStopMotion.delaySeconds * 10;
    runTimeConfig.motorStep = dataStopMotion.stepSize;
    runTimeConfig.motorStepMultiplier = 1;
  }
}


bool loadConfig(uint16_t address, uint8_t *dataStart, uint8_t len) {
//  if (EEPROM.read(address++) == ver) {
  for (uint8_t forIterator = 0; forIterator < len; forIterator++) {
    *((char*)dataStart + forIterator) = EEPROM.read(address + forIterator);
    Serial.print("Loaded ");
    Serial.print(*((char*)dataStart + forIterator), DEC);
    Serial.print(" from address ");
    Serial.println(address + forIterator);
  }
    return 1;
//  }
  return 0;
}

void saveConfig(uint16_t address, uint8_t *dataStart, uint8_t len) {
//  EEPROM.write(address++, ver); // write config version
  for (uint8_t forIterator = 0; forIterator < len; forIterator++) {
    Serial.print("Saving ");
    Serial.print(*((char*)dataStart + forIterator), DEC);
    Serial.print(" in address ");
    Serial.println(address + forIterator);
    EEPROM.update(address + forIterator, *((char*)dataStart + forIterator));
  }
}


/*
   Mode/Menu selection
*/

/*
   Drawing functions
*/
void menu::_tftMenuStringPrint(const uint8_t x, const uint8_t y) {
  _tft.setCursor(x, y);
  _tft.print(_string);
}

/*
 * Setting functions
 */
void menu::picsLeftAndTotal() {
  snprintf(_string, 9, "%d/%d", runTimeInfo.picsTaken, runTimeConfig.frames);
  _tft.print(_string);
}

void menu::nextPic() {
  snprintf(_string, 9, "%d", runTimeInfo.nextFrameTime / 100);
  _tft.print(_string);
}

void menu::timeLeft() {
//  Serial.print("Endtime: ");
//  Serial.print(runTimeConfig.endTime);
//  Serial.print(" millis(): ");
//  Serial.println(millis());
//  uint32_t tmpVar = (runTimeConfig.endTime - millis()) / 1000UL;
//  if (tmpVar > 21384000UL) { // more than 99 hours - most likely overflow occurred
//    Serial.print("Millis: ");
//    Serial.println(millis());
//    Serial.print("endTime overflow by ");
//    Serial.print((millis() - runTimeConfig.endTime) / 1000);
//    Serial.println(" seconds");
////    tmpVar = 0;
//  }
//  snprintf(_string, 9, "%02lu:%02lu:%02lu", tmpVar / 60 / 60, tmpVar / 60 % 60, tmpVar % 60 );
  uint32_t tmpVar = runTimeInfo.endTime / 100;
  snprintf(_string, 9, "%02lu:%02lu:%02lu", tmpVar / 60 / 60, tmpVar / 60 % 60, tmpVar % 60 );
  _tft.print(_string);
}

int menu::limitMinCB() {
  return 0;
}

int menu::limitMaxCB() {
  return 100;
}

int menu::currentPositionCB() {
  return _currentPositionData;
}

void menu::_stopMoDelaySet(int value) {
  if (value >= 30) {
    value -= value % 5;
    _encoder.setStep(5);
  } else {
    _encoder.setStep(1);
  }
  dataStopMotion.delaySeconds = value;
}

void menu::_stopMoDelayPrint() {
  snprintf(_string, 9, "%d", dataStopMotion.delaySeconds);
  _tft.print(_string);
}

int menu::_limitMinZero() {
  return 0;
}

int menu::_limitMinOne() {
  return 0;
}

int menu::_stopMoDelayMax() {
  return 5;
}

int menu::_stopMoDelayCurrent() {
  return dataStopMotion.delaySeconds;
}

void menu::_stopMoStepSet(int value) {
  dataStopMotion.stepSize = value;
}
void menu::_stopMoStepPrint() {
  if (dataStopMotion.delaySeconds == 0) {
    dataStopMotion.delaySeconds = 1;
    dataStopMotion.stepSize = 1;
  }
  snprintf(_string, 9, "%d", dataStopMotion.stepSize);
  _tft.print(_string);
  return ;
}
int menu::_stopMoStepMax() {
  return 10;
}
int menu::_stopMoStepCurrent() {
  return dataStopMotion.stepSize;
}


/*
 * Time lapse menu functions
 */
void menu::_tlTimeSet(int value) {
  if (value >= 600) {
    value -= value % 60;
    _encoder.setStep(60);
  } else if (value >= 160) {
    value -= value % 30;
    _encoder.setStep(30);
  } else if (value >= 30) {
    value -= value % 5;
    _encoder.setStep(5);
  } else {
    _encoder.setStep(1);
  }
  // Minimum time lapse time is 12 minutes (TLMINTIME), (minimal step, 1 second * 70 cm slider length, 80 max)
  if (value < TLMINTIME) {
    value = TLMINTIME;
  }
  dataTL.realTimeMinutes = value;
  if (dataTL.lengthSeconds) {
    _recalcTLLength();
  }
}
void menu::_tlTimePrint() {
  if (!dataTL.realTimeMinutes) { // Default setting for time lapse
    dataTL.motorStepCount = 4;
    dataTL.realTimeMinutes = TLMINTIME;
    dataTL.lengthSeconds = 10;
    _recalcTLLength();
  }
  snprintf(_string, 9, "%dh %dmin",  dataTL.realTimeMinutes / 60, dataTL.realTimeMinutes % 60);
  _tft.print(_string);
}
int menu::_intMax() {
  return 32767;
}
int menu::_tlTimeCurrent() {
  return dataTL.realTimeMinutes;
}
void menu::_tlFramesPrint() {
  snprintf(_string, 9, "%d",  dataTL.frames);
  _tft.print(_string);
}
int menu::_tlFramesCurrent() {
  return dataTL.frames;
}

void menu::_recalcTLLength() {
  dataTL.frames = SLIDER_MAX_STEPS / dataTL.motorStepCount;
  dataTL.lengthSeconds = dataTL.frames / 25;
  dataTL.waitTime = (uint16_t) ((dataTL.realTimeMinutes * 60L * 10L) / dataTL.frames); // * 10 is to get 100 ms granularity
  dataTL.picPerMin = dataTL.frames / dataTL.realTimeMinutes;
  
  Serial.print(" step: ");
  Serial.print(dataTL.motorStepCount);
  Serial.print(" frames: ");
  Serial.print(dataTL.frames);
  Serial.print(" lengthSeconds: ");
  Serial.print(dataTL.lengthSeconds);
  Serial.print(" WaitTime: ");
  Serial.println(dataTL.waitTime);
}
void menu::_tlLengthSet(int value) {
  if ((uint16_t) value > dataTL.lengthSeconds) {
    if (dataTL.motorStepCount > 1) {
      dataTL.motorStepCount--;
    }
  } else if ((uint16_t) value < dataTL.lengthSeconds) {
    if (dataTL.motorStepCount < 10) {
      dataTL.motorStepCount++;
    }
  }

  _recalcTLLength();
}
void menu::_tlLengthPrint() {
  snprintf(_string, 9, " %dm:%ds", dataTL.lengthSeconds / 60, dataTL.lengthSeconds % 60);
  _tft.print(_string);
}
int menu::_tlLengthCurrent() {
  return dataTL.lengthSeconds;
}

void menu::_tlStartInSet(int value) {
  if (value >= 600) {
    value -= value % 60;
    _encoder.setStep(60);
  } else if (value >= 160) {
    value -= value % 30;
    _encoder.setStep(30);
  } else if (value >= 30) {
    value -= value % 5;
    _encoder.setStep(5);
  } else {
    _encoder.setStep(1);
  }
  dataTL.startInMinutes = value;
}
void menu::_tlStartInPrint() {
  snprintf(_string, 9, "%dh:%dmin",  dataTL.startInMinutes / 60, dataTL.startInMinutes % 60);
  _tft.print(_string);
}
int menu::_tlStartInCurrent() {
  return dataTL.startInMinutes;
}

void menu::_tlPicMinuteSet(int value) {
  dataTL.picPerMin = value;
}
void menu::_tlPicMinutePrint() {
  snprintf(_string, 9, "%d",  dataTL.picPerMin);
  _tft.print(_string);
}
int menu::_tlPicMinuteCurrent() {
  return dataTL.picPerMin;
}

/*
 * Progressive Time Lapse menu functions
 */
void menu::_tlpModeTrigger(int) {
  dataTLPro.increase = !dataTLPro.increase;

  if (dataTLPro.increase) {
    dataTLPro.initialStep = 10;
    if (dataTLPro.startPicPerMin < dataTLPro.endPicPerMin) {
      tmpVar2 = dataTLPro.startPicPerMin;
      dataTLPro.startPicPerMin = dataTLPro.endPicPerMin;
      dataTLPro.endPicPerMin = tmpVar2;
    }
    if (dataTLPro.waitTimeIncrease < 0) {
      dataTLPro.waitTimeIncrease *= -1;
    }
  } else {
    dataTLPro.initialStep = dataTLPro.decreasingInitialStep;
    if (dataTLPro.startPicPerMin > dataTLPro.endPicPerMin) {
      tmpVar2 = dataTLPro.startPicPerMin;
      dataTLPro.startPicPerMin = dataTLPro.endPicPerMin;
      dataTLPro.endPicPerMin = tmpVar2;
    }
    if (dataTLPro.waitTimeIncrease > 0) {
      dataTLPro.waitTimeIncrease *= -1;
    }
  }
}
void menu::_tlpModePrint() {
  if (dataTLPro.increase) {
    snprintf(_string, 9, "Incr");
  } else {
    snprintf(_string, 9, "Decr");
  }
  _tft.print(_string);
}

int menu::_tlpModeCurrent() {
  return dataTLPro.increase;
}

void menu::_tlpCalcRealTime() {
  dataTLPro.calculatedRealTime = 0;
  uint16_t frames = dataTLPro.frames;
  uint16_t step = dataTLPro.initialStep;
//  uint8_t iteration = 0;

  // first picture consists of waiting for motor stop (0) + wait for pic + take pic time
  dataTLPro.calculatedRealTime += runTimeConfig.waitBeforePic + runTimeConfig.picReleaseTime;
//  Serial.println("Calculating progressive time lapse");
//  Serial.print("Realtime: ");
//  Serial.print(dataTLPro.realTimeMinutes * 60);
//  Serial.print(" Initial real time: ");
//  Serial.print(dataTLPro.calculatedRealTime);
//  Serial.print(" frames ");
//  Serial.print(dataTLPro.frames);
//  Serial.print(" initialStep ");
//  Serial.print(dataTLPro.initialStep);
//  Serial.print(" waitTimeIncrease ");
//  Serial.println(dataTLPro.waitTimeIncrease);
  dataTLPro.startPicPerMin = 0;
  dataTLPro.endPicPerMin = 0;
  
  while(--frames) {
    step += dataTLPro.waitTimeIncrease;
    dataTLPro.calculatedRealTime += step; // Normunds
//    Serial.print("calculatedRealTime: ");
//    Serial.print(dataTLPro.calculatedRealTime);
//    Serial.print(" step: ");
//    Serial.print(step);
//    Serial.print(" framesLeft: ");
//    Serial.print(frames);
//    Serial.print(" iteration: ");
//    Serial.println(iteration++);
  }

  // calculate startPicPerMin and endPicPerMin
  step = dataTLPro.initialStep;
  for (forIterator = dataTLPro.initialStep / 10; forIterator <= dataTLPro.calculatedRealTime; forIterator += step) {
    if (forIterator <= 600L) {
      dataTLPro.startPicPerMin++;
    }
    if (forIterator >= dataTLPro.calculatedRealTime - 600L) {
      dataTLPro.endPicPerMin++;
    }
    step += dataTLPro.waitTimeIncrease;
  }
  Serial.print(" initial step: ");
  Serial.print(dataTLPro.initialStep);  
  Serial.print(" wait time increase: ");
  Serial.print(dataTLPro.waitTimeIncrease);
  Serial.print(" calculatedRealTime(sec * 10): ");
  Serial.println(dataTLPro.calculatedRealTime);
  
}

void menu::_tlpTimeSet(int value) {
  if (value >= 600) {
    value -= value % 60;
    _encoder.setStep(60);
  } else if (value >= 160) {
    value -= value % 30;
    _encoder.setStep(30);
  } else if (value >= 30) {
    value -= value % 5;
    _encoder.setStep(5);
  } else {
    _encoder.setStep(1);
  }
  if (value < 1) {
    value = 1;
  }
  // calculate longest progressive time lapse possible within this time limit
  // afterwards we can shorten time lapse by making frames less frequent
  dataTLPro.frames = 0;
  dataTLPro.waitTimeIncrease = 1;
  dataTLPro.realTimeMinutes = value;
  if (!dataTLPro.motorStepCount) {
    dataTLPro.motorStepCount = 1;
  }

  // get frame count and correct step
  calcTLPro();

  // get real time and start/end pics/min
  _tlpCalcRealTime();

  if (dataTLPro.increase) {
    dataTLPro.initialStep = 10;
  } else {
    dataTLPro.initialStep = dataTLPro.decreasingInitialStep;
    dataTLPro.waitTimeIncrease *= -1;
    if (dataTLPro.startPicPerMin > dataTLPro.endPicPerMin) {
      tmpVar2 = dataTLPro.startPicPerMin;
      dataTLPro.startPicPerMin = dataTLPro.endPicPerMin;
      dataTLPro.endPicPerMin = tmpVar2;
    }
  }
  dataTLPro.lengthSeconds = dataTLPro.frames / 25;
}
void menu::calcTLPro() {
  calcTLProVars.stepsDone = 0;
  calcTLProVars.totalFrames = 0;
  calcTLProVars.calcMotorStepMultiplier = 1;
  calcTLProVars.totalTime = 10; // tenths of a second
  calcTLProVars.currentFramePause = 10; // initial step
//  Serial.println("calcTLPro()");

  while (1) {
    calcTLProVars.totalFrames++;
//    Serial.println(calcTLProVars.totalFrames);
    calcTLProVars.stepsDone += dataTLPro.motorStepCount;
    calcTLProVars.currentFramePause += dataTLPro.waitTimeIncrease;
    calcTLProVars.totalTime += calcTLProVars.currentFramePause;
    if (calcTLProVars.stepsDone > SLIDER_MAX_STEPS) {
//      Serial.println("StepsDone Reached");
      dataTLPro.waitTimeIncrease++;
      calcTLPro();
      return;
    }
    if (calcTLProVars.totalFrames > SLIDER_MAX_STEPS) {
//      Serial.println("calcTLProVars.totalFrames Reached");
      dataTLPro.waitTimeIncrease++;
      calcTLPro();
      return;
    }
    if (calcTLProVars.totalTime >= (dataTLPro.realTimeMinutes * 60UL * 10UL)) {
//      Serial.println("Found acceptable range");
      while (calcTLProVars.stepsDone * ((calcTLProVars.calcMotorStepMultiplier + 1)/ 10.0) < SLIDER_MAX_STEPS) {
        calcTLProVars.calcMotorStepMultiplier++;
//        Serial.println(calcTLProVars.calcMotorStepMultiplier);
//        Serial.println((calcTLProVars.calcMotorStepMultiplier/ 10.0));
//        Serial.println((float) calcTLProVars.stepsDone * (calcTLProVars.calcMotorStepMultiplier/ 10.0));
//        printf("calcMotorStepMultiplier: %d\n", calcTLProVars.calcMotorStepMultiplier);
      }
      dataTLPro.decreasingInitialStep = calcTLProVars.currentFramePause;
//        dataTLPro.motorStepCount = calcTLProVars.calcMotorStepMultiplier;
      dataTLPro.frames = calcTLProVars.totalFrames;
      //calcTLProVars.stepsDone *= calcTLProVars.calcMotorStepMultiplier / 10.0;
//            printf("Found: frameTimeIncrease: %d, frames: %d, total time: %d:%d, distance travelled: %d\n", frameTimeIncrease, calcTLProVars.totalFrames,
//                calcTLProVars.totalTime / 10 / 60 / 60, calcTLProVars.totalTime / 10 / 60 % 60, calcTLProVars.stepsDone);
      //printf("Stats: calcsExeced %d stepLimitReached %d frameLimitReached %d\n", stats.calcsExeced, stats.stepLimitReached, stats.frameLimitReached);
      return;
    }
  }
}

void menu::_tlpTimePrint() {
  snprintf(_string, 9, "%dh:%dmin",  dataTLPro.realTimeMinutes / 60, dataTLPro.realTimeMinutes % 60);
  _tft.print(_string);
}
int menu::_tlpTimeMin() {
  return 30; // 30 minutes min
}
int menu::_tlpTimeMax() {
  return 60 * 99; // 99 hours max
}
int menu::_tlpTimeCurrent() {
  dataTLPro.motorStepCount = 1; // set other settings as default when user modifies this setting
  return dataTLPro.realTimeMinutes;
}

void menu::_tlpLengthSet(int value) {
  value *= -1;
  uint8_t prev = dataTLPro.motorStepCount;
  uint8_t lengthSeconds;
  // recalculate till length changes in user interface
  do {
    lengthSeconds = dataTLPro.lengthSeconds;
    if (value > prev && dataTLPro.motorStepCount < 10) {
      dataTLPro.motorStepCount++;
    } else if (value < prev && dataTLPro.motorStepCount > 1) {
      dataTLPro.motorStepCount--;
    }
    _tlpTimeSet(dataTLPro.realTimeMinutes);
//    Serial.print("motorStepCount: ");
//    Serial.println(dataTLPro.motorStepCount);
  } while (lengthSeconds == dataTLPro.lengthSeconds && (dataTLPro.motorStepCount != 1 && dataTLPro.motorStepCount != 10));
  _encoder.setStep(1); // the previous call messes with encoder steps, correct this here
}
void menu::_tlpLengthPrint() {
  snprintf(_string, 9, " %dm:%ds", dataTLPro.lengthSeconds / 60, dataTLPro.lengthSeconds % 60);
  _tft.print(_string);
}
int menu::_tlpLengthMin() {
  return -10;
}
int menu::_tlpLengthMax() {
  return -1;
}
int menu::_tlpLengthCurrent() {
  Serial.print("_tlpLengthCurrent(): ");
  Serial.println(dataTLPro.motorStepCount * -1);
  return dataTLPro.motorStepCount * -1;
}

void menu::_tlpStartInSet(int value) {
  if (value >= 600) {
    value -= value % 60;
    _encoder.setStep(60);
  } else if (value >= 160) {
    value -= value % 30;
    _encoder.setStep(30);
  } else if (value >= 30) {
    value -= value % 5;
    _encoder.setStep(5);
  } else {
    _encoder.setStep(1);
  }
  dataTLPro.startInMinutes = value;
}
void menu::_tlpStartInPrint() {
  snprintf(_string, 9, " %dh:%dm", dataTLPro.startInMinutes / 60, dataTLPro.startInMinutes % 60);
  _tft.print(_string);
}
int menu::_tlpStartInMax() {
  return 32767;
}
int menu::_tlpStartInCurrent() {
  return dataTLPro.startInMinutes;
}

void menu::_tlpLinearitySet(int value) {
  (void) value;
}
void menu::_tlpLinearityPrint() {
  
}
int menu::_tlpLinearityMax() {
  return 32767;
}
int menu::_tlpLinearityCurrent() {
  return 0;
}

void menu::_tlpFramesPrint() {
  snprintf(_string, 9, "%d",  dataTLPro.frames);
  _tft.print(_string);
}
void menu::_tlpFirstMinPrint() {
  snprintf(_string, 9, " %d", dataTLPro.startPicPerMin);
  _tft.print(_string);
}
void menu::_tlpLastMinPrint() {
  if (!dataTLPro.realTimeMinutes) { // Default setting
    dataTLPro.waitTimeIncrease = 1;
    _tlpTimeSet(60);
  }
  snprintf(_string, 9, " %d", dataTLPro.endPicPerMin);
  _tft.print(_string);
}

void menu::picsTaken() {
  Serial.print("Pics Taken: ");
  Serial.println(runTimeInfo.picsTaken);
  snprintf(_string, 9, " %d", runTimeInfo.picsTaken);
  _tft.print(_string);
}

void menu::videoLength() {
  snprintf(_string, 9, " %ds + %d", runTimeInfo.picsTaken / 25, runTimeInfo.picsTaken % 25);
  _tft.print(_string);
}

void menu::_saveDataSlider() {
  saveConfig(0, (uint8_t *) &dataSlider, sizeof(dataSlider_t));
}

void menu::_loadDataSlider() {
  loadConfig(0, (uint8_t *) &dataSlider, sizeof(dataSlider_t));
}

void menu::_saveDataTL() {
  /*
   * _tlTimeSet   uint16_t dataTL.realTimeMinutes
   * _tlLengthSet  uint8_t dataTL.motorStepCount (called from _tlTimeSet)
   * _tlStartInSet uint16_t dataTL.startInMinutes do not call nothing
   */
  saveConfig(2, (uint8_t *) &dataTL.realTimeMinutes, sizeof(uint16_t));
  saveConfig(4, (uint8_t *) &dataTL.motorStepCount, sizeof(uint8_t));
  saveConfig(5, (uint8_t *) &dataTL.startInMinutes, sizeof(uint16_t));
}

void menu::_loadDataTL() {
  loadConfig(2, (uint8_t *) &dataTL.realTimeMinutes, sizeof(uint16_t));
  loadConfig(4, (uint8_t *) &dataTL.motorStepCount, sizeof(uint8_t));
  loadConfig(5, (uint8_t *) &dataTL.startInMinutes, sizeof(uint16_t));
  _tlTimeSet(dataTL.realTimeMinutes);
  _recalcTLLength();
}

void menu::_saveDataTLPro() {
  /*
   * _tlpModeTrigger   uint8_t dataTLPro.increase (do not call anything)
   * _tlpLengthSet   uint8_t dataTLPro.motorStepCount (call before _tlTimeSet) - nothing to call here
   * _tlpTimeSet     uint16_t dataTLPro.realTimeMinutes (call <--)
   * _tlpStartInSet    uint16_t dataTLPro.startInMinutes - nothing to call
   */
  saveConfig(7, (uint8_t *) &dataTLPro.increase, sizeof(uint8_t));
  saveConfig(8, (uint8_t *) &dataTLPro.motorStepCount, sizeof(uint8_t));
  saveConfig(9, (uint8_t *) &dataTLPro.realTimeMinutes, sizeof(uint16_t));
  saveConfig(11, (uint8_t *) &dataTLPro.startInMinutes, sizeof(uint16_t));
}

void menu::_loadDataTLPro() {
  loadConfig(7, (uint8_t *) &dataTLPro.increase, sizeof(uint8_t));
  loadConfig(8, (uint8_t *) &dataTLPro.motorStepCount, sizeof(uint8_t));
  loadConfig(9, (uint8_t *) &dataTLPro.realTimeMinutes, sizeof(uint16_t));
  loadConfig(11, (uint8_t *) &dataTLPro.startInMinutes, sizeof(uint16_t));
  _tlpTimeSet(dataTLPro.realTimeMinutes);
}

bool menu::_initializeEeprom() {
  if (EEPROM.read(2047) != 31) {
    Serial.println("EEPROM not written. Writing defaults");
    dataSlider.speed = 25;
    dataSlider.repeat = 0;
  
    dataTL.motorStepCount = 4;
    dataTL.realTimeMinutes = TLMINTIME;
    dataTL.lengthSeconds = 10;
    _recalcTLLength();
  
    dataTLPro.waitTimeIncrease = 1;
    _tlpTimeSet(60);

    _saveDataSlider();
    _saveDataTL();
    _saveDataTLPro();

    EEPROM.write(2047, 31);
    return 1;
  }
  return 0;
}


void printRunTimeInfo(volatile runTimeInfo_t * ri) {
  Serial.println(F("[RunTimeInfo]"));
  Serial.print(F("picsTaken: "));
  Serial.print(ri->picsTaken);
  Serial.print(F(" nextFrameTime: "));
  Serial.print(ri->nextFrameTime);
  Serial.print(F(" framesLeft: "));
  Serial.println(ri->framesLeft);
  Serial.print(F(" endTime: "));
  Serial.println(ri->endTime);
  Serial.print(F(" repeat: "));
  Serial.print(ri->repeat);
  Serial.print(F(" startTimeLeft: "));
  Serial.println(ri->startTimeLeft);
}

void printRunTimeInfo(runTimeInfo_t * ri) {
  Serial.println(F("[RunTimeInfo]"));
  Serial.print(F("picsTaken: "));
  Serial.print(ri->picsTaken);
  Serial.print(F(" nextFrameTime: "));
  Serial.print(ri->nextFrameTime);
  Serial.print(F(" framesLeft: "));
  Serial.println(ri->framesLeft);
  Serial.print(F(" endTime: "));
  Serial.println(ri->endTime);
  Serial.print(F(" repeat: "));
  Serial.print(ri->repeat);
  Serial.print(F(" startTimeLeft: "));
  Serial.println(ri->startTimeLeft);
}

void printRunTimeConfig(runTimeConfig_t * rc) {
  Serial.println(F("[RunTimeConfig]"));
  Serial.print(F(" sliderMode "));
  Serial.print(rc->sliderMode);
  Serial.print(F(" frames "));
  Serial.print(rc->frames);
  Serial.print(F(" timeBetweenFrames "));
  Serial.print(rc->timeBetweenFrames);
  Serial.print(F(" motorWaitIncreaseStep "));
  Serial.print(rc->motorWaitIncreaseStep);
  Serial.print(F(" motorStep "));
  Serial.print(rc->motorStep);
  Serial.print(F(" motorStepMultiplier "));
  Serial.print(rc->motorStepMultiplier);
  Serial.print(F(" waitBeforePic "));
  Serial.print(rc->waitBeforePic);
  Serial.print(F(" picReleaseTime "));
  Serial.print(rc->picReleaseTime);
  Serial.print(F(" speed "));
  Serial.print(rc->speed);
  Serial.print(F(" startTime "));
  Serial.print(rc->startTime);
  Serial.print(F(" endTime "));
  Serial.println(rc->endTime);
}

