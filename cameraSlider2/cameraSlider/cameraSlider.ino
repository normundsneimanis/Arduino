#include <EEPROM.h>
#include "RGBLed.h"
#include "TimeLapse.h"
#include "timedelay.h"
#include "HBridgeMotor.h"
//#include "RotaryResistanceSensor.h"
#include "rotaryEncoder.h"
#include "TwoSevenSegLEDs.h"
#include "userInput.h"
#include "hallSensorInput.h"
#include <TimerOne.h>
#include "TimeLapse.h"
#include "cameraSliderController.h"
//#include "cheatCode.h"

// Arduino Pro mini pinout
// rotary encoder
#define encoderPinLeft 2
#define encoderPinRight 15
#define encoderButtonPin 17

#define batterySensor A7

// 7-segment LED control
#define clockResetPin 16
#define clockPin 4
#define clockPin2 12
// Motor direction control
#define motorPinGoLeft 3
#define motorPinGoRight 11
// Analog input pins
//#define potentiometerPin 15
// 18 is A4
// 19 is A5
// 20 is A6
// 21 is A7
#define hallSensorLeft 18
#define hallSensorRight 19
// Digital input pins
// (button) Pin to detect if movement should be repeated
#define repeatPin 9
#define timeLapsePin 7
#define directionSwitchLeft 8
#define directionSwitchRight 14
// Time Lapse configuration buttons - digital.
//#define configuringTLDrivingTimePin 16
// 17
//#define configuringTLWaitTimePin 17
// TimeLapse configuration status RGB LED connections
#define TLRedPin 5
#define TLGreenPin 6
#define TLBluePin 10
#define cameraTriggerPin 13

// EEPROM save adresses for Time Lapse settings
#define SliderSpeedSaveAddr 0
#define TLWaitTimeSaveAddr 1
#define TLRunTimeSaveAddr 2
#define TLIncreasingRateSaveAddr 3
#define TLDecreasingRateSaveAddr 4

#define sleeptime 100
#define timeLapseSleepTime 50
#define interrupt100ms 100000
byte doTimeLapse = -1;
HBridgeMotorDirection motorDirection = HBM_STOP;
HBridgeMotorDirection motorDirectionPrev;
// Does slider repeats sliding or stops when reaches the end
byte repeatSlideInProgress = 0;

timedelay StatisticsDelay(330); // How frequently to print statistics
timedelay TLDelay(0);
timedelay TLShowConfigDelay(800);
byte showWhichConfig = 0;
byte blinkRedLedState = LOW;
TwoSevenSegLEDs segmentLeds(clockPin, clockPin2, clockResetPin);
HBridgeMotor motor(motorPinGoLeft, motorPinGoRight);
//RotaryResistanceSensor pot(potentiometerPin);
userInput userInput(directionSwitchLeft, directionSwitchRight, repeatPin);
hallSensorInput hallSensor(hallSensorLeft, hallSensorRight);
RGBLed rgbled(TLRedPin, TLGreenPin, TLBluePin);
cameraSliderController controller;

rotaryEncoder encoder(encoderPinLeft, encoderPinRight, encoderButtonPin);

// currently running state so we can initialize properly
byte currentState = 0; // 0 - no state set, 1 - slider, 2 - time lapse
byte sliderRunning = 0; // used to know when to save slider speed to eeprom
int sliderSpeed = 0; // Speed that is used on usual camera slider

// Time Lapse
TimeLapse timeLapse(TLWaitTimeSaveAddr, TLRunTimeSaveAddr, TLIncreasingRateSaveAddr, TLDecreasingRateSaveAddr);
byte TLWaitTime = timeLapse.eepromReadWait();
byte TLRunTime = timeLapse.eepromReadRun();
byte TLWaitTimeChanged = 0;
byte TLRunTimeChanged = 0;
byte TLRunning = 0;
timeLapseState TLState = TLOff;
HBridgeMotorDirection prevDirection;
byte configuringTimeLapse = 0; // 0 - off, 1 - configuring run time, 2 - wait time, 3 - configuring progressive

volatile byte startMotor = 0;
unsigned long TLMotorRunTime;
volatile int calledManageMotor = 0;
int pictureTakenOverflows = 0;

float batteryVoltage = 0;
timedelay batteryVoltageDelay(30000);
timedelay batteryVoltageMeasurementDelay(3000);
byte lowBattery = 0;

//#define MAX_TIMER_SETS 200
//unsigned long timerSets[MAX_TIMER_SETS];
//int timerSetsIndex = 0;
//int timerSetsOverflows = 0;

void setTimer1(unsigned long timer) {
  noInterrupts();
//  timerSets[timerSetsIndex++] = timer;
//  if (timerSetsIndex == MAX_TIMER_SETS) {
//    timerSetsIndex = 0;
//    timerSetsOverflows++;
//  }
  Timer1.setPeriod(timer);
  interrupts();
}

//void printTimer1Sets() {
//  if ((!timerSetsIndex) && (!timerSetsOverflows)) {
//    return;
//  }
//  noInterrupts();
//  Serial.print(F("Timer1 overflows: "));
//  Serial.print(timerSetsOverflows);
//  Serial.print(F(" data: "));
//  for (int i = 0; i < timerSetsIndex; i++) {
//    Serial.print(i);
//    Serial.print(F(": "));
//    Serial.print(timerSets[i]);
//    timerSetsIndex = 0;
//  }
//  Serial.println(F("."));
//  interrupts();
//}

// Restarts program from beginning but does not reset the peripherals and registers
void softwareReset() {
  asm volatile ("  jmp 0");  
}

// this is ISR for starting and stopping motor after specified amount of time
// used for TimeLapse
void manageMotor(void) {
	calledManageMotor++;
	if (startMotor == 1) {
		motor.start();
		setTimer1(TLMotorRunTime);
		startMotor = 2;
	} else if (startMotor == 2) {
		motor.stop();
		startMotor = 0;
	}
}

// ISR to aggregate rotary encoder movement
void rotaryEncoderISR() {
	if (digitalRead(encoderPinLeft) == digitalRead(encoderPinRight)) {
		encoder.stepUp();
	} else {
		encoder.stepDown();
	}
}

// ISR to blink red led when time lapse is completed
void blinkRedLed(void) {
	if (blinkRedLedState == LOW) {
		blinkRedLedState = HIGH;
		rgbled.red();
	} else {
		blinkRedLedState = LOW;
		rgbled.off();
	}
}

void checkBatteryVoltage() {
	int reading = analogRead(batterySensor);
	batteryVoltage = (float) (1330.0 * (reading * 5.0 / 1023.0) / 330);
	if (reading < 610) {
		lowBattery = 1;
	}
}

void lowBatteryWarning() {
	rgbled.red();
	delay(10);
	rgbled.off();
	delay(50);
	rgbled.red();
	delay(10);
	rgbled.off();
	delay(50);
	rgbled.red();
	delay(10);
	rgbled.off();
	delay(50);
	rgbled.red();
	delay(10);
	rgbled.off();
	delay(50);
	rgbled.red();
	delay(10);
	rgbled.off();
	delay(50);
	rgbled.red();
	delay(10);
	rgbled.off();
	delay(50);
	rgbled.red();
	delay(10);
	rgbled.off();
	delay(50);
}

void setup() {
	Serial.begin(115200);
	segmentLeds.init();
	motor.init();
	//pot.init();
	userInput.init();
	hallSensor.init();
	rgbled.init();
	timeLapse.init();
	controller.init();

	encoder.init();
	attachInterrupt(digitalPinToInterrupt(encoderPinLeft), rotaryEncoderISR, CHANGE);

	sliderSpeed = EEPROM.read(SliderSpeedSaveAddr);

	// Time Lapse enable
	pinMode(timeLapsePin, INPUT);
	Timer1.initialize(interrupt100ms / 10); // Run ISR every 0.01 second

	pinMode(batterySensor, INPUT);
	checkBatteryVoltage();
}


void loop() {
	StatisticsDelay.cycle();
	batteryVoltageDelay.cycle();
	batteryVoltageMeasurementDelay.cycle();
	// Read all input sensors/buttons
	// pot.readSensor();
	doTimeLapse = digitalRead(timeLapsePin);

	hallSensor.readSensors();
	userInput.readSensors();

	controller.setUserInput(userInput.getDirection());
	controller.setRepeatEnable(userInput.repeatEnabled());
	controller.sliderLeftReached(hallSensor.leftReached());
	controller.sliderRightReached(hallSensor.rightReached());
	motor.setDirection(controller.getDirection());

	if (StatisticsDelay.hasPassed()) {
		StatisticsDelay.reset();
		Serial.print(F(" batVolt: "));
		Serial.print(batteryVoltage);
		Serial.print(F(" Led: "));
		Serial.print(segmentLeds.getLed());
		Serial.print(F(" speed: "));
		Serial.print(motor.getSpeed());
		Serial.print(F(" UI Dir: "));
		Serial.print(userInput.getDirection());
		Serial.print(F(" UI Rep: "));
		Serial.print(userInput.repeatEnabled());
		Serial.print(F(" H1: "));
		Serial.print(hallSensor.leftReached());
		Serial.print(F(" H2: "));
		Serial.print(hallSensor.rightReached());
		Serial.print(F(" H1 raw: "));
		Serial.print(hallSensor.getRawLeft());
		Serial.print(F(" H2 raw: "));
		Serial.print(hallSensor.getRawRight());
		Serial.print(F(" Dir: "));
		Serial.print(controller.getDirection());
		Serial.print(" TLState: ");
		Serial.print(TLState);
		Serial.print(" TLRunTime: ");
		Serial.print(TLRunTime);
		Serial.print(" TLWaitTime: ");
		Serial.print(TLWaitTime);
		Serial.print(" TLDelay (0.1s): ");
		Serial.print(TLDelay.getTime() / 100);
		Serial.print(" TLMotorRunTime: ");
		Serial.print(TLMotorRunTime);
		Serial.print(F(" MM: "));
		Serial.println(calledManageMotor);
		//printTimer1Sets();
	}
  
	if (doTimeLapse || currentState == 2) {
		// init time lapse
		if (currentState != 2) {
			motor.smoothEnable(0);
			encoder.setStep(1);
			currentState = 2;
		}
		// All time lapse control functionality goes here
		// Enter Wait time configuration mode, if requested.
		if (encoder.buttonPressed()) {
			configuringTimeLapse = timeLapse.configButtonPressed();
		}
		while (configuringTimeLapse == 1) {
			if (!TLRunTimeChanged) {
				TLRunTimeChanged = 1;
				encoder.setCurrentPosition(TLRunTime);
			}
			rgbled.green();
			encoder.setLimits(TLMinWaitTime, TLMaxWaitTime);
			TLRunTime = encoder.getCurrentPosition();
			segmentLeds.setLed(TLRunTime);
			if (encoder.buttonPressed()) {
				configuringTimeLapse = timeLapse.configButtonPressed();
			} else {
				delay(timeLapseSleepTime);
			}
		}

		if (TLRunTimeChanged) {
			TLRunTimeChanged = 0;
			rgbled.off();
			timeLapse.setRunTime(TLRunTime);
			timeLapse.eepromWriteRun();
		}
		while (configuringTimeLapse == 2) {
			if (!TLWaitTimeChanged) {
				TLWaitTimeChanged = 1;
				encoder.setCurrentPosition(TLWaitTime);
			}
			rgbled.blue();
			encoder.setLimits(TLMinRunTime, TLMaxRunTime);
			TLWaitTime = encoder.getCurrentPosition();
			segmentLeds.setLed(TLWaitTime);
			if (encoder.buttonPressed()) {
				configuringTimeLapse = timeLapse.configButtonPressed();
			} else {
				delay(timeLapseSleepTime);
			}
		}
		if (TLWaitTimeChanged) {
			TLWaitTimeChanged = 0;
			rgbled.off();
			timeLapse.setWaitTime(TLWaitTime);
			timeLapse.eepromWriteWait();
			TLDelay.setTime(TLWaitTime * 1000);
			TLDelay.reset();
		}

		if (configuringTimeLapse > 2) {
			// 3 - mode, increasing or decreasing. 4 - configure step
			// First, enter mode - increasing, right or decreasing - left
			if (configuringTimeLapse == 3) {
				encoder.setLimits(0, 2);
				encoder.setStep(1);
				encoder.setCurrentPosition(1);
				rgbled.red();
				while (1) {
					if (encoder.buttonPressed() && encoder.getCurrentPosition() != 1) {
						if (encoder.getCurrentPosition() == 2) {
							timeLapse.setCheatType(TLProgressiveIncreasing);
						} else {
							timeLapse.setCheatType(TLProgressiveDecreasing);
						}
						configuringTimeLapse = timeLapse.configButtonPressed();
						break;
					}
					segmentLeds.setLed(encoder.getCurrentPosition());
					delay(timeLapseSleepTime);
				}
			} else if (configuringTimeLapse == 4) {
				// enter progressive amount
				encoder.setStep(1);
				if (timeLapse.getCheatType() == TLProgressiveIncreasing) {
					encoder.setLimits(1, 99);
					rgbled.green();
					encoder.setCurrentPosition(timeLapse.getIncreasingTime());
				} else if (timeLapse.getCheatType() == TLProgressiveDecreasing) {
					encoder.setLimits(1, 30);
					rgbled.blue();
					encoder.setCurrentPosition(timeLapse.getDecreasingTime());
				}
				while (1) {
					if (encoder.buttonPressed()) {
						if (timeLapse.getCheatType() == TLProgressiveIncreasing) {
							timeLapse.setIncreasingTime(encoder.getCurrentPosition());
						} else if (timeLapse.getCheatType() == TLProgressiveDecreasing) {
							timeLapse.setDecreasingTime(encoder.getCurrentPosition());
						}
						configuringTimeLapse = timeLapse.configButtonPressed();
						break;
					}
					segmentLeds.setLed(encoder.getCurrentPosition());
					delay(timeLapseSleepTime);
				}
			}
		}

		// if direction given
		if (userInput.getDirection() != UI_STOP) {
			if (!TLRunning) {
				rgbled.off();
				TLDelay.setTime(TLWaitTime * 1000);
				TLDelay.reset();
				timeLapse.clearPicturesTaken();
				TLState = TLWaitingForStart;
				segmentLeds.setLed(0);
				prevDirection = controller.getDirection();
				TLRunning = 1;
				pictureTakenOverflows = 0;
			}
			TLDelay.cycle();
			switch (TLState) {
			case TLOff:
				break;
			case TLWaitingForStart:
				if (TLDelay.hasPassed()) {
					TLDelay.reset();
					TLMotorRunTime = (interrupt100ms * TLRunTime) / 4;
					motor.setSpeed(255);
					noInterrupts();
					setTimer1(interrupt100ms / 10);
					Timer1.attachInterrupt(manageMotor);
					TLState = TLMotorRunning;
					startMotor = 1;
					interrupts();
				} else if (batteryVoltageMeasurementDelay.hasPassed()) {
					batteryVoltageMeasurementDelay.reset();
					checkBatteryVoltage();
				} else if (lowBattery && batteryVoltageDelay.hasPassed()) {
					batteryVoltageDelay.reset();
					lowBatteryWarning();
				}
				break;
			case TLMotorRunning:
				// check if motor has to be stopped due to end of rail, transfer to timeLapseDone if reached
				if (startMotor == 0 || controller.getDirection() == HBM_STOP) {
					Timer1.detachInterrupt();
					motor.stop();
					TLState = TLMotorDone;
				} else if (prevDirection != controller.getDirection()) {
					// if direction changed while motor is running, turn the motor around immediately
					motor.stop();
					motor.start();
					prevDirection = controller.getDirection();
				}
				break;
			case TLMotorDone:
				// light led more times if picture overflow happened
				for (int i = 0; i < ((timeLapse.getPicturesTaken() / 100) + 1); i++) {
					if (i > 0) {
						rgbled.off();
						delay(50);
					}
					rgbled.green();
					delay(TLGreenTime);
				}
				rgbled.off();
				timeLapse.waitForCamera(TLCameraDelayTime - ((timeLapse.getPicturesTaken() / 100) * 60));
				digitalWrite(cameraTriggerPin, HIGH);
				timeLapse.waitForButton();
				digitalWrite(cameraTriggerPin, LOW);
				timeLapse.pictureTaken();
				segmentLeds.setLed(timeLapse.getPicturesTaken() % 100);
				if (timeLapse.cheatConfigured()) {
					if (timeLapse.getCheatType() == TLProgressiveDecreasing) {
						if ((TLDelay.getTime() / 1000) > 3) {
							TLDelay.setTime(TLDelay.getTime() - (timeLapse.getDecreasingTime() * 33));
						}
					} else if (timeLapse.getCheatType() == TLProgressiveIncreasing) {
						TLDelay.setTime(TLDelay.getTime() + (timeLapse.getIncreasingTime() * 33));
					}
				}
				TLState = TLWaitingForStart;
				batteryVoltageMeasurementDelay.reset();
				if (controller.getDirection() == HBM_STOP) {
					TLState = TLDone;
				}
				break;
			case TLDone:
				setTimer1(interrupt100ms * 8);
				Timer1.attachInterrupt(blinkRedLed);
				TLState = TLOff;
				break;
			}
		} else {
			motor.stop();
			if (TLRunning) {
				TLRunning = 0;
				TLState = TLOff;
				Timer1.detachInterrupt();
				softwareReset();
			}
			TLShowConfigDelay.cycle();
			if (TLShowConfigDelay.hasPassed()) {
				TLShowConfigDelay.reset();
				if (batteryVoltageMeasurementDelay.hasPassed()) {
					batteryVoltageMeasurementDelay.reset();
					checkBatteryVoltage();
				}
				if (showWhichConfig) {
					showWhichConfig = 0;
					if (timeLapse.cheatConfigured()) {
						rgbled.red();
						delay(50);
					}
					rgbled.blue();
					segmentLeds.setLed(TLWaitTime);
				} else {
					showWhichConfig = 1;
					rgbled.green();
					segmentLeds.setLed(TLRunTime);
				}
			}
		}
		if (!lowBattery) {
			delay(timeLapseSleepTime);
		}
	} else {
		/*
		 * Normal camera slider functionality follows
		 */
		if (currentState != 1) {
			motor.smoothEnable(1);
			encoder.setLimits(0, 100);
			encoder.setStep(5);
			encoder.setCurrentPosition(sliderSpeed);
			rgbled.off();
			currentState = 1;
		}
		if ((!sliderRunning) && motor.isRunning()) {
			sliderRunning = 1;
			if (EEPROM.read(SliderSpeedSaveAddr) != encoder.getCurrentPosition()) {
				EEPROM.write(SliderSpeedSaveAddr, encoder.getCurrentPosition());
			}
		} else if ((sliderRunning) && (!motor.isRunning())) {
			sliderRunning = 0;
			batteryVoltageMeasurementDelay.reset();
		}
		segmentLeds.setLed(encoder.getCurrentPosition());

		motor.setSpeed(encoder.getCurrentPosition() * 2.55); // converts 0-100 to 0-255 for analogWrite
		motor.commit();

		if (!sliderRunning && batteryVoltageMeasurementDelay.hasPassed()) {
			batteryVoltageMeasurementDelay.reset();
			checkBatteryVoltage();
		}

		if (lowBattery && batteryVoltageDelay.hasPassed()) {
			batteryVoltageDelay.reset();
			lowBatteryWarning();
		} else {
			delay(sleeptime);
		}
	}
}
