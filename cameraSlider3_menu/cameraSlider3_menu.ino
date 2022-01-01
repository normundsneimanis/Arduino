#include "hallSensorInput.h"
#include "timedelay.h"
#include "rotaryEncoder.h"
#include "HBridgeMotor.h"
#include "menu.h"
#include "images.h"
#include "userInput.h"
#include "hallSensorInput.h"
#include "cameraSliderController.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <Adafruit_GFX.h>    // Core graphics library

//#define DEBUG 1

/*
 * Sorry about spagetti code
 */


/* Arduino Pro Mini ports
#define cs_sd   10
#define cs_lcd   9
#define dc       8
#define rst      7
*/

/* Slider Controller Atmega644 */
// RST PB2  2
// CS  PB4  4
// DC  PB3  3
// Moved to menu.h
//#define cs_lcd   4
//#define dc       3
//#define rst      2

/* 
 *  Camera photo requests
 *    focus  PA1 25
 *    pic    PA2 26
 */
#define cameraTriggerFocus 25
#define cameraTriggerPic 26


/* Camera photo trigger 
 *  PA3 27
 *  PA4 28
 */
#define triggerFocus 27
#define triggerPic 28

/* Slider direction requests 
 *  left PC4 20
 *  right PC5 21

*/
#define directionSwitchLeft 20
#define directionSwitchRight 21

/* Analog input pins
 *  sens1 PA5 29
 *  sens2 PA6 30
*/
#define hallSensorLeft 29
#define hallSensorRight 30

/* rotary encoder 
 *  button - PC3 19
 *  right - PC2 18
 *  left - PC1 17

*/
#define encoderPinLeft 17
#define encoderPinRight 18
#define encoderButtonPin 19

/* 
 *  Voltage sensor
 *  PA0
 */
#define batterySensor 24

/*
 * Motor
 */
#define motorPwmPin 14
#define motorDir1 13
#define motorDir2 12

Adafruit_ST7735 tft = Adafruit_ST7735(cs_lcd, dc, rst);
timedelay StatisticsDelay(2000); // How frequently to print statistics
timedelay batteryCheckDelay(260000); // When to read battery after slider has stopped
timedelay startInDelay(1000);
uint8_t batteryCheckPending = 1;
rotaryEncoder encoder(encoderPinLeft, encoderPinRight, encoderButtonPin);
userInput userInput(directionSwitchLeft, directionSwitchRight);
menu menu(tft, encoder, userInput);
HBridgeMotor motor(motorPwmPin, motorDir1, motorDir2);

volatile uint16_t ISRCalled = 0;

/*
 * SD card currently is not working
 * 
 * Connections:
 * Arduino -> Display (port)
 * 7 -> R 1K -> RST 1
 * 8 -> R 1K -> D/C 3
 * 9 -> R 1K -> CS 2
 * 11 -> R 1K -> DIN 4
 * 13 -> R 1K -> CLK 5
 */


int cameraTriggerState = 0;

// states used for slider controlling periodic ISR
typedef enum sliderAction {
    MOTOR_START_WAIT,
    MOTOR_STOP_WAIT,
    PIC_START_WAIT,
    PIC_STOP_WAIT,
    SLIDERACTION_MAX,
} sliderAction_t;

sliderAction_t getNextAction();

const char * const sliderActionString[]  = {
    "MOTOR_START_WAIT",
    "MOTOR_STOP_WAIT",
    "PIC_START_WAIT",
    "PIC_STOP_WAIT",
    "SLIDERACTION_MAX",
};

volatile extern runTimeInfo_t runTimeInfo;
extern runTimeConfig_t runTimeConfig;
volatile runTimeInfo_t * runTimeInfo_ptr1 = &runTimeInfo;
runTimeInfo_t * runTimeInfo_ptr = const_cast<runTimeInfo_t *>(runTimeInfo_ptr1);

runTimeInfo_t runTimeInfo2;
runTimeConfig_t runTimeConfig2;
sliderAction_t sliderCurrentAction;

//uint8_t frames = 0;
//uint16_t waitTime = 20;
//uint8_t increaseStep = 0;

volatile uint32_t currentWaitTime = 0;
volatile uint32_t totalActualWaits = 0;
volatile uint8_t currentWaitTimeOverflowed = 0;

typedef struct stats {
  uint32_t motorStarts;
  uint32_t motorStops;
  uint32_t picStarts;
  uint32_t picStops;
} stats_t;
stats_t stats;

uint16_t cameraStabilized = 0;
uint32_t getCurrentWaitTime() {
    if (runTimeConfig.sliderMode == 1 || runTimeConfig.sliderMode == 2) { // Time lapse or Progressive time lapse
        if (sliderCurrentAction == MOTOR_START_WAIT) {
            if (runTimeConfig.sliderMode == 2) {
              runTimeConfig.timeBetweenFrames += runTimeConfig.motorWaitIncreaseStep; // if progressive time lapse
            }
            runTimeInfo.nextFrameTime = (runTimeConfig.timeBetweenFrames + 1) * 10;
            // convert PictureWaitTime from 10s of second to 100ths of second
            uint32_t pictureWaitTime = runTimeConfig.motorStep * runTimeConfig.motorStepMultiplier +
                runTimeConfig.waitBeforePic * 10UL +
                runTimeConfig.picReleaseTime * 10UL; // wait time for motor, picture and picture button release
//            Serial.print("pictureWaitTime: ");
//            Serial.println(pictureWaitTime);
            
            if (((runTimeConfig.timeBetweenFrames * 10UL) - pictureWaitTime) > 700) { // if we have more than 10 seconds
              cameraStabilized = 500;
              return ((runTimeConfig.timeBetweenFrames * 10UL) - pictureWaitTime - cameraStabilized);
            } else if ((runTimeConfig.timeBetweenFrames * 10UL) - pictureWaitTime > 200) {
              cameraStabilized = ((runTimeConfig.timeBetweenFrames * 10UL) - pictureWaitTime) / 2 +
                ((runTimeConfig.timeBetweenFrames * 10UL) - pictureWaitTime) % 2;
//              Serial.print("Smart stabilizing camera: ");
//              Serial.println(cameraStabilized);
//              Serial.print("runTimeConfig.timeBetweenFrames: ");
//              Serial.println(runTimeConfig.timeBetweenFrames);
              return ((runTimeConfig.timeBetweenFrames * 10UL) - pictureWaitTime - cameraStabilized);
            } else {
              return ((runTimeConfig.timeBetweenFrames * 10UL) - pictureWaitTime);
            }
        } else if (sliderCurrentAction == MOTOR_STOP_WAIT) {
            return runTimeConfig.motorStep * runTimeConfig.motorStepMultiplier; // motor step
        } else if (sliderCurrentAction == PIC_START_WAIT) {
          if (cameraStabilized) {
            return (runTimeConfig.waitBeforePic * 10UL + cameraStabilized); // wait till picture taken
          } else {
            return runTimeConfig.waitBeforePic * 10; // wait till picture taken
          }
        } else if (sliderCurrentAction == PIC_STOP_WAIT) {
          cameraStabilized = 0;
          return runTimeConfig.picReleaseTime * 10; // press picture button this long time
        }
    } else if (runTimeConfig.sliderMode == 3) { // stop motion
      if (sliderCurrentAction == MOTOR_START_WAIT) {
        return runTimeConfig.timeBetweenFrames * 10; // no motor start wait for StepMo
      } else if (sliderCurrentAction == MOTOR_STOP_WAIT) {
        return runTimeConfig.motorStep * runTimeConfig.motorStepMultiplier; // motor step size for stopmo
      } else if (sliderCurrentAction == PIC_START_WAIT) {
        return runTimeConfig.waitBeforePic * 10;
      } else if (sliderCurrentAction == PIC_STOP_WAIT) {
        return runTimeConfig.picReleaseTime * 10;
      }
    }
    Serial.println("FAIL");
    while(1) {}
    return 0;
}


sliderAction_t getNextAction() {
    sliderAction_t tmpState = sliderCurrentAction;
    sliderCurrentAction = (sliderAction_t) ((sliderAction_t) (sliderCurrentAction + 1) % SLIDERACTION_MAX);
    return tmpState;
}

volatile uint8_t sliderStopped = 0;

// here goes periodic interrupt that controls what motor or camera does
volatile uint32_t printFlag = 0;
volatile sliderAction_t currentState;
volatile sliderAction_t currentStateTmp;
volatile uint16_t hallSensor1;
volatile uint16_t hallSensor2;

volatile uint8_t isrInProgress = 0;
volatile uint8_t isrRequests = 0;

void readHallSensors() {
  ADMUX = bit (REFS0) | bit (MUX2) | bit (MUX0); // ADC5, right sensor
  bitSet(ADCSRA, ADSC);  // start a conversion
  while (bit_is_set(ADCSRA, ADSC))
    { }
  hallSensor1 = ADC;
  
  ADMUX = bit (REFS0) | bit (MUX2) | bit (MUX1); // ADC6, left sensor
  bitSet(ADCSRA, ADSC);  // start a conversion
  while (bit_is_set(ADCSRA, ADSC))
    { }
  hallSensor2 = ADC;
}

void sliderISR() {
  printFlag++;
  
  if (runTimeInfo.startTimeLeft) {
    runTimeInfo.startTimeLeft--;
    if (runTimeInfo.startTimeLeft % 100 == 0) {
      menu.infoUpdated();
    }
    if (runTimeInfo.startTimeLeft == 0) {
      menu.startInFinished();
      menu.infoUpdated();
    }
    return;
  }
  

  // read hall sensors
  if (printFlag % 10 == 0) {
    readHallSensors();
  }

  if (currentStateTmp != currentState) {
    Serial.print("currentAction: ");
    Serial.print(sliderActionString[currentState]);
    Serial.print(" ");
    Serial.println(currentWaitTime);
    currentStateTmp = currentState;
  }
  
  if (runTimeConfig.sliderMode != 0) {
    if (--currentWaitTime == 0) {
      if (currentState == MOTOR_START_WAIT) {
        motor.setDirection(userInput.getDirection());
        motor.commit();
      } else if (currentState == MOTOR_STOP_WAIT) {
        motor.stop();
        if (runTimeConfig.sliderMode == 3) {
          menu.infoUpdated();
          stopISR();
          return;
        }
      } else if (currentState == PIC_START_WAIT) {
        PORTA |= (1 << PORTA3);
        PORTA |= (1 << PORTA4);
      } else if (currentState == PIC_STOP_WAIT) {
        // stop taking picture
        PORTA &= ~(1 << PORTA3);
        PORTA &= ~(1 << PORTA4);
        runTimeInfo.picsTaken++;
        Serial.print("totalActualWaits: ");
        Serial.println(totalActualWaits);
        if (runTimeConfig.sliderMode == 1 || runTimeConfig.sliderMode == 2) {
          runTimeInfo.framesLeft--;
          if (runTimeInfo.framesLeft == 0) {
            menu.sliderFinished();
            stopISR();
            return;
          }
        }
      }
      currentWaitTime = getCurrentWaitTime();
      currentState = getNextAction();
      
      if (currentWaitTime < 655350) {
        totalActualWaits += currentWaitTime;
      } else {
//        memcpy(&runTimeInfo2, &runTimeInfo_ptr, sizeof(runTimeInfo));
//        memcpy(&runTimeConfig2, &runTimeConfig, sizeof(runTimeConfig));
        currentWaitTimeOverflowed = 1;
        stopISR();
        return;
      }
      if (currentState == MOTOR_START_WAIT) {
        stats.motorStarts += currentWaitTime;
      } else if (currentState == MOTOR_STOP_WAIT) {
        stats.motorStops += currentWaitTime;
      } else if (currentState == PIC_START_WAIT) {
        stats.picStarts += currentWaitTime;
      } else if (currentState == PIC_STOP_WAIT) {
        stats.picStops += currentWaitTime;
      }

      // Update info also if nextFrameTime is always under 99
      if (currentState == PIC_STOP_WAIT && runTimeInfo.nextFrameTime < 99) {
        menu.infoUpdated();
      }
      
      runTimeInfo.nextFrameTime--;
      runTimeInfo.endTime--;
    } else {
        runTimeInfo.nextFrameTime--;
        runTimeInfo.endTime--;
        if (runTimeInfo.nextFrameTime % 100 == 0) {
          menu.infoUpdated();
        }
    }
    // stop if end is reached
    if ((userInput.getDirection() == UI_RIGHT && hallSensor1 < 100) ||
    (userInput.getDirection() == UI_LEFT && hallSensor2 < 100)) {
      menu.sliderFinished();
      stopISR(); // stop ISR
    }
  } else if (runTimeConfig.sliderMode == 0) {
    // if slider
    if ((motor.getDirection() == UI_RIGHT && hallSensor1 < 100) || 
    (motor.getDirection() == UI_LEFT && hallSensor2 < 100)) {
      if (runTimeInfo.repeat) {
        motor.reverse();
        runTimeInfo.repeat--;
      } else {
        motor.stop();
      }
    } else {
      motor.setSpeed(runTimeConfig.speed);
      motor.commit();
    }
    // start as soon as we know that we have slider enabled
    // if requested direction is available, start motor
    // else if (requested direction sensor is on) stop motor and notofiy that we have finished
    // else notify that we have finished
  }
}

volatile uint8_t largestISRQueue = 0;
ISR(TIMER1_COMPA_vect) {
  isrRequests++;
  while(1) {
    cli();
    if (!isrInProgress && isrRequests) {
      isrRequests--;
      isrInProgress = 1;
      sei();
      if (isrRequests > largestISRQueue) {
        largestISRQueue = isrRequests;
      }
      sliderISR();
      cli();
      isrInProgress = 0;
      sei();
    } else {
      sei();
      break;
    }
  }
}

ISR (PCINT2_vect) {
  if (digitalRead(encoderPinLeft) == digitalRead(encoderPinRight)) {
    encoder.stepUp();
  } else {
    encoder.stepDown();
  }
  if (runTimeConfig.sliderMode == 0) {
    menu.runEncoder(encoder.getCurrentPosition());
  }
//  rotaryEncoderISR();
}

volatile uint8_t testIsrEnabled = 0;
void testIsr() {
  while(testIsrEnabled) {
    sliderISR();
  }
}

void stopISR() {
  TCCR1A = 0;
  TCCR1B = 0;
  // stop motors
  motor.stop();
  // stop triggering camera
  PORTA &= ~(1 << PORTA3);
  PORTA &= ~(1 << PORTA4);
  StatisticsDelay.setTime(1048575);
  sliderStopped = 1;
  testIsrEnabled = 0;
  batteryCheckPending = 1;

  // Print current data
  Serial.println("stopISR() RunTimeInfo:");
  printRunTimeInfo(&runTimeInfo);
  Serial.println("stopISR() RunTimeConfig");
  printRunTimeConfig(&runTimeConfig);
}

void startISR() {
  // MOTOR_STOP_WAIT if time lapse or progressive time lapse, 
  // PIC_START_WAIT if slowMo
  currentStateTmp = (sliderAction_t) 9;
  cameraStabilized = 0;
  if (runTimeConfig.sliderMode == 1 || runTimeConfig.sliderMode == 2) {
    currentState = sliderCurrentAction = PIC_START_WAIT;
    motor.smoothEnable(0);
    motor.setSpeed(255);
    runTimeInfo.picsTaken = 0;
  } else if (runTimeConfig.sliderMode == 3) {
    currentState = sliderCurrentAction = PIC_START_WAIT;
    motor.smoothEnable(0);
    motor.setSpeed(255);
  } else {
    motor.smoothEnable(1);
    motor.setDirection(userInput.getDirection());
  }
  largestISRQueue = 0;
  currentWaitTime = 1;

  runTimeInfo.nextFrameTime = runTimeConfig.timeBetweenFrames * 10;
  runTimeInfo.framesLeft = runTimeConfig.frames;

  printFlag = 0;
  totalActualWaits = 0;

  StatisticsDelay.setTime(1000);
  batteryCheckPending = 0;

  // Print current data
  Serial.println("startISR() RunTimeInfo:");
  printRunTimeInfo(&runTimeInfo);
  Serial.println("startISR() RunTimeConfig");
  printRunTimeConfig(&runTimeConfig);

  // initial sensor read
  readHallSensors();

#ifdef DEBUG
  testIsrEnabled = 1;
  testIsr();
#else
  TCCR1A = 0;
  TCCR1B = 0; // Stop timer so we can modify it
  TCCR1B |= (1 << WGM12); // Configure timer 1 for CTC mode
  OCR1A   = 1250 - 1; // Set CTC compare value to 100Hz at 8MHz AVR clock, with a prescaler of 64
  //OCR1B   = 12500 - 1; // Set CTC compare value to 10Hz at 8MHz AVR clock, with a prescaler of 64
  // 2500 - 1 for 10ms for 16MHz
  TIMSK1 |= (1 << OCIE1A);// | (1 << OCIE1B); // Enable CTC interrupt
  sei(); //  Enable global interrupts
  TCCR1B |= ((1 << CS11) | (1 << CS10)); // Set up timer at Fcpu/64
#endif
}

void setup()
{
  Serial.begin(115200);
  Serial.println(F("Initializing ports"));

  pinMode(cameraTriggerFocus, INPUT);
  digitalWrite(cameraTriggerFocus, HIGH);
  pinMode(cameraTriggerPic, INPUT);
  digitalWrite(cameraTriggerPic, HIGH);
  pinMode(triggerFocus, OUTPUT);
  pinMode(triggerPic, OUTPUT);

  pinMode(directionSwitchLeft, INPUT);
  digitalWrite(directionSwitchLeft, HIGH);
  pinMode(directionSwitchRight, INPUT);
  digitalWrite(directionSwitchRight, HIGH);

  pinMode(hallSensorLeft, INPUT);
  pinMode(hallSensorRight, INPUT);

  pinMode(batterySensor, INPUT);

  encoder.init();
  userInput.init();
  userInput.readSensors();
  menu.init();
  checkBattery();
  motor.init();
  StatisticsDelay.setTime(1048575);
//  attachInterrupt(digitalPinToInterrupt(encoderPinLeft), rotaryEncoderISR, CHANGE); // CHANGE RISING FALLING
  PCMSK2 |= (1 << PCINT17); /* Enable PCINT17 */
  PCICR |= (1 << PCIE2); /* Activate interrupt on enabled PCINT23-16 */
  sei();                    // turn on interrupts

  // set up ADC
  ADCSRA = 0;  // disable ADC
  ADCSRA =  bit (ADEN);   // turn ADC on
  ADCSRA |= bit (ADPS2);  // Prescaler of 16

}

void checkBattery() {
  menu.updateBatteryStatus(map(analogRead(batterySensor), 0, 1023, 0, 2650));
}

void loop() {
  StatisticsDelay.cycle();
  if (batteryCheckPending) {
    if (batteryCheckPending == 1) {
      batteryCheckPending = 2;
      batteryCheckDelay.cycle();
      batteryCheckDelay.reset();
    } else if (batteryCheckPending == 2) {
      batteryCheckDelay.cycle();
      if (batteryCheckDelay.hasPassed()) {
        Serial.println("Checking battery");
        batteryCheckPending = 1;
        checkBattery();
      }
    }
  }

  if (StatisticsDelay.hasPassed()) {
    StatisticsDelay.reset();
    Serial.print("ISR cycles: ");
    Serial.print(printFlag);
    Serial.print(" speed: ");
    Serial.print(runTimeConfig.speed);
    Serial.print(" picsTaken: ");
    Serial.print(runTimeInfo.picsTaken);
    Serial.print(" framesTotal: ");
    Serial.print(runTimeConfig.frames);
    Serial.print(" timeBetweenFrames: ");
    Serial.println(runTimeConfig.timeBetweenFrames);
    Serial.print(" motorStep: ");
    Serial.print(runTimeConfig.motorStep);
    Serial.print(" waitBeforePic: ");
    Serial.print(runTimeConfig.waitBeforePic);
    Serial.print(" PicReleaseTime: ");
    Serial.print(runTimeConfig.picReleaseTime);
    Serial.print(" currentAction: ");
    Serial.print(sliderActionString[sliderCurrentAction]);
    Serial.print(" S1: ");
    Serial.print(hallSensor1);
    Serial.print(" S2: ");
    Serial.println(hallSensor2);
  }

  if (sliderStopped) {
    sliderStopped = 0;
    Serial.print("MotorStarts: ");
    Serial.print(stats.motorStarts);
    Serial.print(" MotorStops: ");
    Serial.print(stats.motorStops);
    Serial.print(" picStarts: ");
    Serial.print(stats.picStarts);
    Serial.print(" picStops: ");
    Serial.print(stats.picStops);
    
    Serial.print(" totalActualWaits: ");
    Serial.println(totalActualWaits);
    printRunTimeInfo(&runTimeInfo);
    printRunTimeConfig(&runTimeConfig);
    Serial.print("LargestISRQueue: ");
    Serial.print(largestISRQueue);
    Serial.print(" endTime: ");
    Serial.print(runTimeInfo.endTime);
    Serial.print(" ISR cycles: ");
    Serial.println(printFlag);
  }
  
  if (currentWaitTimeOverflowed) {
    Serial.println("CurrentWaitTime overflowed!!!");
    Serial.println(currentWaitTime);
    StatisticsDelay.setTime(1048575);
    currentWaitTimeOverflowed = 0;
    Serial.println("RunTimeInfo:");
    printRunTimeInfo(&runTimeInfo);
    Serial.println("RunTimeConfig");
    printRunTimeConfig(&runTimeConfig);
    Serial.print(" currentAction: ");
    Serial.print(sliderActionString[sliderCurrentAction]);
    while(1) {}
  }

  // step motion - read focus and picture request buttons
  if (runTimeConfig.sliderMode == 3) {
    if (!digitalRead(cameraTriggerFocus)) {
      Serial.println("Triggering Focus");
      digitalWrite(triggerFocus, HIGH);
    } else {
      digitalWrite(triggerFocus, LOW);
    }
    
    if (!digitalRead(cameraTriggerPic)) {
      Serial.println("Triggering Picture");
      startISR();
      delay(600);
    }
  }

  menu.inputUpdate();
  menu.draw();

//  delay(50);
}


