#include <stdint.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include "timedelay.h"


// resistance 3 * 1R + 2.2R + 3.3R with parallel LED, 330R + LED resistance
#define RESISTANCE 8.3

#define encoderPinLeft 5
#define encoderPinRight 6
#define encoder0ButtonPin 7

#define buttonPin encoderPinLeft

#define analogSelect0 2
#define analogSelect1 3
#define analogSelect2 4


typedef enum {
	S_DISCONNECTED,
	S_CHARGING1,
	S_DISCHARGING1,
	S_CHARGING2,
	S_DISCHARGING2,
	S_STORECHARGE,
	S_DONE,
} bateryStates_t;

typedef enum {
	C_AUTOTEST_S,
	C_AUTOTEST_D,
	C_CHARGE,
	C_DISCHARGE,
	C_STORE,
	C_NUMCHARGERSTATES,
} chargerStates_t;

const char * const chargerStatesString[] = {
	"Chrg/Dischrg/Store single",
	"Chrg/Dischrg/Store double",
	"Charge mode",
	"Discharge mode",
	"Store mode",
};

volatile chargerStates_t chargerState = C_AUTOTEST_S;
volatile chargerStates_t chargerStatePrev = C_NUMCHARGERSTATES; // Initial state for correct entry

#define NUMBATTERIES 8
// Voltage to detect battery with - 2V
#define MINVOLTAGE 200
// charge to 4.14 V
#define CHARGEVOLTAGE 414
// if charging does not go to CHARGEVOLTAGE, start discharging after
#define MINCHARGEVOLTAGE 405
#define MINCHARGEOVERTIME (90 * 60)

// discharge to 2.5 V
#define DISCHARGEVOLTAGE 250
// Storage charge to 3.8V
#define STORECHARGE 385

#define REFVOLTAGE 431

// Time to charge after charging has reached CHARGEVOLTAGE, in seconds
#define OVERCHARGETIME (30 * 60)

// print string buffer array size
#define PRINTMAX 30

#define cs_lcd   10
#define dc       9
#define rst      8

// Interrupt frequency, Hz
#define ISR_FREQ 10
// time to wait between reading battery status, in ISR ticks
#define TIMEBETWEENREADS (60 * ISR_FREQ)
// time to wait to make reading after swtiching off charging, in ISR Ticks
#define READWAIT (1 * ISR_FREQ)
#define BUTTON_DEBOUNCE_TIME (ISR_FREQ / 3)

// ADC interrupt related variables
// http://www.avrfreaks.net/forum/tut-c-newbies-guide-avr-adc?page=all
#define ADC_ARRAY_MAX 3
volatile uint16_t ADCResultsArray[ADC_ARRAY_MAX];
volatile uint8_t ADCArrayIdx;

//Pin connected to latch pin (ST_CP) of 74HC595
const int latchPin = A1;
//Pin connected to clock pin (SH_CP) of 74HC595
const int clockPin = A2;
////Pin connected to Data in (DS) of 74HC595
const int dataPin = A3;

static inline void disableCharging(volatile uint16_t * portsEnabled, uint8_t port);
static inline void enableCharging(volatile uint16_t * portsEnabled, uint8_t port);
static inline void disableDischarging(volatile uint16_t * portsEnabled, uint8_t port);
static inline void enableDischarging(volatile uint16_t * portsEnabled, uint8_t port);

typedef struct {
	volatile bateryStates_t batteryState;
	volatile uint16_t chargeCompleteWait;
	volatile uint16_t batteryVoltage;
	volatile double batteryCapacity[2];
	volatile uint8_t statusUpdated;
	volatile uint16_t batteryChargeTime[2];
	volatile uint16_t batteryDischargeTime[2];
	volatile uint16_t stateChanges;
	volatile uint16_t readDelayTick;
	volatile uint16_t readTick;
	volatile uint16_t minChargeOvertime;
} battery_t;


bateryStates_t currentState;
timedelay statisticsDelay(1000);

// port number used as iterator in interrupt service routine
volatile uint8_t sprt;
volatile uint16_t portsEnabled;

/*
 * I want screen to be as responsive as possible, that means that I
 * need to read analog inputs in periodic real-time manner, every 50ms
 * After voltage have been updated, I want to run state machine as soon as
 * possible and then screen updates can happen in background
 * 
 * in mainloop, we need to notify screen on what have been updated
 */
// Start with defining information placement on the screen
// The proceed with routine that take placement, clears the area and re-draws
// the information for specific battery

typedef struct {
	uint8_t vx; // Voltage X coordinate
	uint8_t vy; // Voltage Y
	uint8_t cx; // Capacity X
	uint8_t cy; // Capacity Y
} textLocations_t;


volatile uint16_t buttonInterruptTicks;

static const uint8_t debugArrayMax = 100;
volatile uint8_t debugArray[debugArrayMax];
volatile uint8_t debugArrayIndex;

void debugArrayAdd2(uint8_t from, uint8_t stateTo) {
	if (debugArrayIndex >= debugArrayMax) {
		return;
	}
	debugArray[debugArrayIndex++] = from;
	debugArray[debugArrayIndex++] = stateTo;
}

void debugArrayAdd1(uint8_t val) {
	if (debugArrayIndex >= debugArrayMax) {
		return;
	}
	debugArray[debugArrayIndex++] = val;
}

void debugArrayPrint2() {
	Serial.println("Debug array: ");
	for (int i = 0; i < debugArrayIndex; i += 2) {
		Serial.print(i/2);
		Serial.print(": ");
		Serial.print(debugArray[i]);
		Serial.print(" ");
		Serial.print(debugArray[i+1]);
		Serial.println("");
	}
	debugArrayIndex = 0;
}

void debugArrayPrint1() {
	Serial.println("Debug array: ");
	for (int i = 0; i < debugArrayIndex; i++) {
		Serial.print(i);
		Serial.print(": ");
		Serial.println(debugArray[i]);
	}
	debugArrayIndex = 0;
}


uint8_t hasTickPassed(volatile uint16_t * tickCount) {
	if (*tickCount) {
		*tickCount -= 1;
		if (!(*tickCount)) {
			return 1;
		}
	}
	return 0;
}


void initBattery(volatile battery_t * batt) {
	batt->batteryState = S_DISCONNECTED;
	batt->batteryVoltage = 0;
	batt->stateChanges = 0;
	batt->statusUpdated = 1; // display battery status on startup
	clearBatteryCounters(batt);
}


void clearBatteryCounters(volatile battery_t * batt) {
	batt->chargeCompleteWait = 0;
	batt->batteryCapacity[0] = 0.0;
	batt->batteryCapacity[1] = 0.0;
	batt->batteryChargeTime[0] = 0;
	batt->batteryChargeTime[1] = 0;
	batt->batteryDischargeTime[0] = 0;
	batt->batteryDischargeTime[1] = 0;
	batt->readDelayTick = 0;
	batt->readTick = 0;
	batt->minChargeOvertime = 0;
}

volatile unsigned int invalidState;
void printBatteryStatus(volatile battery_t * batt) {
	Serial.print(F("batteryVoltage: "));
	Serial.print(batt->batteryVoltage);
	Serial.print(F(". batteryState: "));
	switch (batt->batteryState) {
	case S_DISCONNECTED:
		Serial.print(F("S_DISCONNECTED"));
		break;
	case S_CHARGING1:
		Serial.print(F("S_CHARGING1"));
		break;
	case S_DISCHARGING1:
		Serial.print(F("S_DISCHARGING1"));
		break;
	case S_CHARGING2:
		Serial.print(F("S_CHARGING2"));
		break;
	case S_DISCHARGING2:
		Serial.print(F("S_DISCHARGING2"));
		break;
	case S_STORECHARGE:
		Serial.print(F("S_STORECHARGE"));
		break;
	case S_DONE:
		Serial.print(F("S_DONE"));
		break;
	default:
		invalidState++;
	}
	if (batt->stateChanges) {
		Serial.print(F(". stateChanges: "));
		Serial.print(batt->stateChanges);
	}
	if (batt->chargeCompleteWait) {
		Serial.print(F(". chargeCompleteWait: "));
		Serial.print(batt->chargeCompleteWait);
	}
	if (batt->batteryCapacity[0]) {
		Serial.print(F(". batteryCapacity[0]: "));
		Serial.print(batt->batteryCapacity[0]);
	}
	if (batt->batteryCapacity[1]) {
		Serial.print(F(". batteryCapacity[1]: "));
		Serial.print(batt->batteryCapacity[1]);
	}
	if (batt->batteryChargeTime[0]) {
		Serial.print(F(". batteryChargeTime[0]: "));
		Serial.print(batt->batteryChargeTime[0]);
	}
	if (batt->batteryChargeTime[1]) {
		Serial.print(F(". batteryChargeTime[1]: "));
		Serial.print(batt->batteryChargeTime[1]);
	}
	if (batt->batteryDischargeTime[0]) {
		Serial.print(F(". batteryDischargeTime[0]: "));
		Serial.print(batt->batteryDischargeTime[0]);
	}
	if (batt->batteryDischargeTime[1]) {
		Serial.print(F(". batteryDischargeTime[1]: "));
		Serial.print(batt->batteryDischargeTime[1]);
	}
	if (batt->readDelayTick) {
		Serial.print(F(". readDelayTick: "));
		Serial.print(batt->readDelayTick);
	}
	if (batt->readTick) {
		Serial.print(F(". readTick: "));
		Serial.print(batt->readTick);
	}
	if (batt->minChargeOvertime) {
		Serial.print(F(". minChargeOvertime: "));
		Serial.print(batt->minChargeOvertime);
	}
	Serial.println(".");
}

volatile battery_t bat[NUMBATTERIES];
Adafruit_ST7735 tft = Adafruit_ST7735(cs_lcd, dc, rst);


// used to clear text from the screen, offsets from vx, vy, cx, cy in pixels
textLocations_t textOffsets = { 50, 9, 110, 9 };

// empty pixels between rows
#define textOffset 1

textLocations_t textLocations[] = {
	{3, textOffset+10-1, 55, textOffset+10-1},
	{3, textOffset+10*2-1, 55, textOffset+10*2-1},
	{3, textOffset+10*3-1, 55, textOffset+10*3-1},
	{3, textOffset+10*4-1, 55, textOffset+10*4-1},
	{3, textOffset+10*5-1, 55, textOffset+10*5-1},
	{3, textOffset+10*6-1, 55, textOffset+10*6-1},
	{3, textOffset+10*7-1, 55, textOffset+10*7-1},
	{3, textOffset+10*8-1, 55, textOffset+10*8-1},
};


void clearArea(uint8_t x, uint8_t y, uint8_t dx, uint8_t dy) {
	tft.fillRect(x, y, dx, dy, ST7735_BLACK);
}


void displayVoltage(uint8_t port, char * text) {
	clearArea(textLocations[port].vx,
		textLocations[port].vy,
		textOffsets.vx,
		textOffsets.vy);
	tft.setCursor(textLocations[port].vx,
		textLocations[port].vy);
	tft.print(text);
}


void displayCapacity(uint8_t port, char * text) {
	clearArea(textLocations[port].cx,
		textLocations[port].cy,
		textOffsets.cx,
		textOffsets.cy);
	tft.setCursor(textLocations[port].cx,
		textLocations[port].cy);
	tft.print(text);
}


void selectPort(unsigned int analogSelect) {
	digitalWrite(analogSelect0, bitRead(analogSelect, 0));
	digitalWrite(analogSelect1, bitRead(analogSelect, 1));
	digitalWrite(analogSelect2, bitRead(analogSelect, 2));
}


void registerWrite(unsigned int portsEnable) {
	digitalWrite(latchPin, LOW);
	shiftOut(dataPin, clockPin, MSBFIRST, portsEnable >> 8);
	shiftOut(dataPin, clockPin, MSBFIRST, portsEnable);
	digitalWrite(latchPin, HIGH);
}


// Selects uC port to read battery voltage and do a reading to stabilize voltage
void selectADC() {
	//ADMUX = bit(REFS0) | bit(MUX2) | bit(MUX1); // ADC6, internal voltage reference
	ADMUX = bit(MUX2) | bit(MUX1); // ADC6, AREF voltage reference
	ADMUX &= ~(bit(REFS0) | bit(REFS1));
	bitSet(ADCSRA, ADSC);  // start a conversion
	while (bit_is_set(ADCSRA, ADSC))
		{ } 
}

void enableADCFreerun() {
	ADCSRA |= (1 << ADIE); // Enable ADC conversion interrupt
	//ADCSRB &= ~(_BV(ADTS2) | _BV(ADTS1) | _BV(ADTS0)); // Do conversions automatically all the time (free-running mode)
	ADCSRB = (ADCSRB & (~(_BV(ADTS2) | _BV(ADTS1) | _BV(ADTS0)) | _BV(ADTS0))) | _BV(ADTS1);
	bitSet(ADCSRA, ADSC);  // start a conversion
}

void disableADCFreerun() {
	ADCSRA &= ~(1 << ADIE); // Disable ADC conversion interrupt
	ADCSRA &= ~(_BV(ADSC)); // Stop conversion (free-running mode)
}



uint16_t readBatteryVoltage(int port) {
	selectPort(port);
	bitSet(ADCSRA, ADSC);  // start a conversion
	while (bit_is_set(ADCSRA, ADSC))
		{ }
	while (bit_is_set(ADCSRA, ADSC))
		{ }
	return (uint16_t) ADC;
}

uint16_t readBatteryVoltageF() {
	uint16_t val = 0;
	uint8_t i;

	cli();
	for (i = 0; i < ADC_ARRAY_MAX; i++) {
		val += ADCResultsArray[i];
	}
	sei();

	return (uint16_t) (val / ADC_ARRAY_MAX);
}


static inline void disableCharging(volatile uint16_t * portsEnabled, uint8_t port) {
	*portsEnabled &= ~(1 << (port));
}


static inline void enableCharging(volatile uint16_t * portsEnabled, uint8_t port) {
	*portsEnabled |= (1 << (port));
	disableDischarging(portsEnabled, port);
}


static inline void disableDischarging(volatile uint16_t * portsEnabled, uint8_t port) {
	*portsEnabled &= ~(1 << (8 + port));
}


static inline void enableDischarging(volatile uint16_t * portsEnabled, uint8_t port) {
	*portsEnabled |= (1 << (8 + port));
	disableCharging(portsEnabled, port);
}


void startISR() {
	cli();
	TCCR1A = 0;
	TCCR1B = 0; // Stop timer so we can modify it
	TCCR1B |= _BV(WGM12); // Configure timer 1 for CTC mode
	//OCR1A   = 3125 - 1; // Set CTC compare value to 20Hz (50ms) at 16MHz AVR clock, with a prescaler of 256
	OCR1A   = 6250 - 1; // Set CTC compare value to 10Hz (100ms) at 16MHz AVR clock, with a prescaler of 256
	TIMSK1 |= _BV(OCIE1A); // Enable CTC interrupt
	TCCR1B |= (_BV(CS12)); // Set up timer at Fcpu/256
	//TCCR1B |= (_BV(CS12) | _BV(CS10)); // Set up timer at Fcpu/1024 for 5Hz frequency
	sei(); //  Enable global interrupts
}


void stopISR() {
	cli();
	TCCR1A = 0;
	TCCR1B = 0; // Stop timer so we can modify it
	TIMSK1 &= ~_BV(OCIE1A); // Disable CTC interrupt
	sei();
}


void configureButtonInterrupt() {
	PCMSK2 |= (1 << PCINT21); /* Enable interrupt on PCINT21 */
	PCICR |= (1 << PCIE2); /* Activate interrupt on enabled PCINT23-16 */
}


// ISR to switch charger modes. Debounced with buttonInterruptTicks.
volatile uint16_t buttonPressed;
ISR(PCINT2_vect) {
	buttonPressed++;
	chargerState = (chargerStates_t) ((chargerState + 1) % C_NUMCHARGERSTATES);
	PCMSK2 &= ~(1 << PCINT21); /* Disable interrupt on PCINT21 */
	buttonInterruptTicks = BUTTON_DEBOUNCE_TIME;
}


void setup() {
	pinMode(analogSelect0, OUTPUT);
	pinMode(analogSelect1, OUTPUT);
	pinMode(analogSelect2, OUTPUT);
	pinMode(buttonPin, INPUT);
	digitalWrite(buttonPin, HIGH);
	pinMode(latchPin, OUTPUT);
	pinMode(dataPin, OUTPUT);
	pinMode(clockPin, OUTPUT);
	registerWrite(0L);
	Serial.begin(115200);

	// set up ADC
	ADCSRA = 0;  // disable ADC
	ADCSRA =  bit(ADEN);   // turn ADC on
	ADCSRA |= bit(ADPS2);  // Prescaler of 16

	tft.initR(INITR_BLACKTAB);
	tft.setRotation(3); // 0 - Portrait, 1 - Lanscape
	tft.setTextWrap(false);
	tft.fillScreen(ST7735_BLACK);
	tft.setTextColor(ST7735_WHITE, ST7735_BLACK);


	selectADC();

	configureButtonInterrupt();
	Serial.println("Starting");
}


void buttonReEnable() {
	if (hasTickPassed(&buttonInterruptTicks)) {
		configureButtonInterrupt();
	}
}


volatile unsigned int isrCalled;
volatile uint8_t rewritePorts;
volatile uint16_t isrRunning;
volatile uint16_t isrOverflow;
ISR(TIMER1_COMPA_vect) {
	isrRunning++;
	isrCalled++;
	if (isrRunning > 1) {
		isrOverflow++;
		isrRunning--;
	} else {
		buttonReEnable();
		if (chargerState == C_AUTOTEST_S || chargerState == C_AUTOTEST_D) {
			autotestISR();
		} else if (chargerState == C_CHARGE) {
			chargeISR();
		} else if (chargerState == C_DISCHARGE) {
			dischargeISR();
		} else if (chargerState == C_STORE) {
			storeChargeISR();
		}
	}
	isrRunning--;
}


uint16_t ADCFreeruns;
ISR(ADC_vect) {
	ADCFreeruns++;
	ADCResultsArray[ADCArrayIdx++] = (uint16_t) ADC;
	if (ADCArrayIdx >= ADC_ARRAY_MAX) {
		ADCArrayIdx = 0;
	}
}


/*
 * Read only voltage so it can be displayed
 */
void chargeISR() {
	for (sprt = 0; sprt < NUMBATTERIES; sprt++) {
		if (isrCalled % ISR_FREQ == 0) {
			bat[sprt].batteryChargeTime[0]++;
		}
		if (hasTickPassed(&(bat[sprt].readTick))) {
			bat[sprt].batteryVoltage = map(readBatteryVoltage(sprt), 0, 1023, 0, REFVOLTAGE);
			bat[sprt].statusUpdated = 1;
			bat[sprt].readDelayTick = TIMEBETWEENREADS;
			enableCharging(&portsEnabled, sprt);
			rewritePorts = 1;
		} else if (hasTickPassed(&(bat[sprt].readDelayTick))) {
			disableCharging(&portsEnabled, sprt);
			bat[sprt].readTick = READWAIT;
			rewritePorts = 1;
		}
	}
	if (rewritePorts) {
		rewritePorts = 0;
		registerWrite(portsEnabled);
	}
}


/*
 * Entry
 *  disable charging/discharging
 *  set timer to wait for reading
 *  clear counters
 *
 * Loop
 *  check timer for reading
 *  if voltage > STORECHARGE, clear timers, discharge
 *  iv voltage < MINVOLTAGE, stop
 */
void dischargeISR() {
	for (sprt = 0; sprt < NUMBATTERIES; sprt++) {
		if (bat[sprt].batteryState == S_DISCONNECTED) {
			bat[sprt].batteryVoltage = map(readBatteryVoltage(sprt), 0, 1023, 0, REFVOLTAGE);
			bat[sprt].statusUpdated = 1;
			if (bat[sprt].batteryVoltage > STORECHARGE) {
				bat[sprt].batteryDischargeTime[0] = 0;
				bat[sprt].batteryCapacity[0] = 0.0;
				bat[sprt].statusUpdated = 1;
				enableDischarging(&portsEnabled, sprt);
				rewritePorts = 1;
				bat[sprt].stateChanges++;
				bat[sprt].batteryState = S_DISCHARGING1;
				bat[sprt].readDelayTick = TIMEBETWEENREADS;
			}
		} else if (bat[sprt].batteryState == S_DISCHARGING1) {
			if (isrCalled % ISR_FREQ == 0) {
				bat[sprt].batteryDischargeTime[0]++;
				bat[sprt].batteryCapacity[0] += ((bat[sprt].batteryVoltage / RESISTANCE) / 60.0 / 6.0);
			}
			if (hasTickPassed(&(bat[sprt].readTick))) {
				bat[sprt].batteryVoltage = map(readBatteryVoltage(sprt), 0, 1023, 0, REFVOLTAGE);
				if (bat[sprt].batteryVoltage < MINVOLTAGE) {
					bat[sprt].batteryState = S_DISCONNECTED;
					bat[sprt].stateChanges++;
					disableDischarging(&portsEnabled, sprt);
					rewritePorts = 1;
					bat[sprt].readTick = 0;
					bat[sprt].readDelayTick = 0;
				} else {
					bat[sprt].statusUpdated = 1;
					bat[sprt].readDelayTick = (TIMEBETWEENREADS / 6);
					enableDischarging(&portsEnabled, sprt);
				}
			} else if (hasTickPassed(&(bat[sprt].readDelayTick))) {
				bat[sprt].readTick = READWAIT / 4;
				disableDischarging(&portsEnabled, sprt);
			}
		} else {
			invalidState++;
		}
	}
	if (rewritePorts) {
		rewritePorts = 0;
		registerWrite(portsEnabled);
	}
}


/*
 * Entry:
 * disable charging
 * set reading delay, reading will be done by ISR
 * 
 * Loop
 * charge for one minute
 * stop charging for 1 second
 * do the reading
 * Start charging again
 */
void storeChargeISR() {
	for (sprt = 0; sprt < NUMBATTERIES; sprt++) {
		if (bat[sprt].batteryState == S_STORECHARGE) {
			if (isrCalled % ISR_FREQ == 0) {
				bat[sprt].batteryChargeTime[0]++;
			}
			if (hasTickPassed(&(bat[sprt].readTick))) {
				bat[sprt].batteryVoltage = map(readBatteryVoltage(sprt), 0, 1023, 0, REFVOLTAGE);
				bat[sprt].readDelayTick = TIMEBETWEENREADS;
				enableCharging(&portsEnabled, sprt);
				bat[sprt].statusUpdated = 1;
				rewritePorts = 1;
			} else if (hasTickPassed(&(bat[sprt].readDelayTick))) {
				disableCharging(&portsEnabled, sprt);
				bat[sprt].readTick = READWAIT;
				rewritePorts = 1;
			}
			if (bat[sprt].batteryVoltage < MINVOLTAGE || bat[sprt].batteryVoltage > STORECHARGE) {
				bat[sprt].stateChanges++;
				bat[sprt].batteryState = S_DISCONNECTED;
				disableCharging(&portsEnabled, sprt);
				rewritePorts = 1;
				bat[sprt].readTick = 0;
				bat[sprt].readDelayTick = 0;
			}
		} else if (bat[sprt].batteryState == S_DISCONNECTED) {
			bat[sprt].batteryVoltage = map(readBatteryVoltage(sprt), 0, 1023, 0, REFVOLTAGE);
			if (bat[sprt].batteryVoltage > MINVOLTAGE && bat[sprt].batteryVoltage < STORECHARGE) {
				enableCharging(&portsEnabled, sprt);
				clearBatteryCounters(&bat[sprt]);
				rewritePorts = 1;
				bat[sprt].statusUpdated = 1;
				bat[sprt].batteryState = S_STORECHARGE;
				bat[sprt].stateChanges++;
				bat[sprt].readDelayTick = TIMEBETWEENREADS;
			}
		} else {
			invalidState++;
		}
	}

	if (rewritePorts) {
		rewritePorts = 0;
		registerWrite(portsEnabled);
	}
}

volatile uint8_t dischargeWhat;
volatile uint8_t isrSecondPassed;
void autotestISR() {
	if (isrCalled % ISR_FREQ == 0) {
		isrSecondPassed = 1;
	} else {
		isrSecondPassed = 0;
	}
	for (sprt = 0; sprt < NUMBATTERIES; sprt++) {
		// Read the voltage
		currentState = bat[sprt].batteryState;
		if (bat[sprt].readDelayTick || bat[sprt].readTick) {
			if (hasTickPassed(&(bat[sprt].readDelayTick))) {
				disableCharging(&portsEnabled, sprt);
				selectPort(sprt);
				bat[sprt].readTick = READWAIT;
				rewritePorts = 1;
			} else if (hasTickPassed(&(bat[sprt].readTick))) {
				bat[sprt].batteryVoltage = map(readBatteryVoltage(sprt), 0, 1023, 0, REFVOLTAGE);
				bat[sprt].statusUpdated = 1;
				if (currentState == S_CHARGING1 ||
						currentState == S_CHARGING2 ||
						currentState == S_STORECHARGE) {
					enableCharging(&portsEnabled, sprt);
					rewritePorts = 1;
					bat[sprt].readDelayTick = TIMEBETWEENREADS;
				} else {
					bat[sprt].readDelayTick = READWAIT / 2;
				}
			}
		} else {
//			disableADCFreerun();
			bat[sprt].batteryVoltage = map(readBatteryVoltage(sprt), 0, 1023, 0, REFVOLTAGE);
//			enableADCFreerun();
			bat[sprt].statusUpdated = 1;
		}
		switch (currentState) {
		case S_DISCONNECTED:
			if (bat[sprt].batteryVoltage > MINVOLTAGE) {
				enableCharging(&portsEnabled, sprt);
				rewritePorts = 1;
				clearBatteryCounters(&bat[sprt]);
				bat[sprt].batteryState = S_CHARGING1;
				bat[sprt].stateChanges++;
				bat[sprt].statusUpdated = 1;
				bat[sprt].readDelayTick = TIMEBETWEENREADS;
			}
			break;
		case S_CHARGING1:
		case S_CHARGING2:
			if (isrSecondPassed) {
				if (currentState == S_CHARGING1) {
					bat[sprt].batteryChargeTime[0]++;
				} else {
					bat[sprt].batteryChargeTime[1]++;
				}
			}
			if (bat[sprt].batteryVoltage > MINCHARGEVOLTAGE) {
				if (isrSecondPassed) {
					bat[sprt].minChargeOvertime++;
					if (bat[sprt].minChargeOvertime > MINCHARGEOVERTIME) {
						// Make sure we switch state to Discharging
						bat[sprt].chargeCompleteWait = OVERCHARGETIME;
						bat[sprt].batteryVoltage = CHARGEVOLTAGE + 1;
					}
				}
			}
			if (bat[sprt].batteryVoltage > CHARGEVOLTAGE) {
				if (bat[sprt].chargeCompleteWait) {
					if (bat[sprt].chargeCompleteWait < OVERCHARGETIME) {
						if (isrSecondPassed) {
							bat[sprt].chargeCompleteWait++;
						}
					} else {
						bat[sprt].chargeCompleteWait = 0;
						enableDischarging(&portsEnabled, sprt);
						rewritePorts = 1;
						if (currentState == S_CHARGING1) {
							bat[sprt].batteryState = S_DISCHARGING1;
						} else if (currentState == S_CHARGING2) {
							bat[sprt].batteryState = S_DISCHARGING2;
						} else {
							invalidState++;
						}
						bat[sprt].readDelayTick = 0;
						bat[sprt].readTick = 0;
						bat[sprt].stateChanges++;
						bat[sprt].statusUpdated = 1;
					}
				} else {
					bat[sprt].chargeCompleteWait = 1;
				}
			} else if (bat[sprt].batteryVoltage < MINVOLTAGE) {
				bat[sprt].batteryState = S_DONE;
				bat[sprt].stateChanges++;
				bat[sprt].statusUpdated = 1;
			}
			break;
		case S_DISCHARGING1:
		case S_DISCHARGING2:
			if (isrSecondPassed) {
				if (currentState == S_DISCHARGING1) {
					dischargeWhat = 0;
				} else {
					dischargeWhat = 1;
				}
				bat[sprt].batteryDischargeTime[dischargeWhat] += 1;
				bat[sprt].batteryCapacity[dischargeWhat] += ((bat[sprt].batteryVoltage / RESISTANCE) / 60.0 / 6.0);
			}
			if (bat[sprt].batteryVoltage < DISCHARGEVOLTAGE) {
				enableCharging(&portsEnabled, sprt);
				rewritePorts = 1;
				if (currentState == S_DISCHARGING1) {
					if (chargerState == C_AUTOTEST_S) {
						bat[sprt].batteryState = S_STORECHARGE;
					} else if (chargerState == C_AUTOTEST_D) {
						bat[sprt].batteryState = S_CHARGING2;
					} else {
						invalidState++;
					}
				} else if (currentState == S_DISCHARGING2) {
					bat[sprt].batteryState = S_STORECHARGE;
				} else {
					invalidState++;
				}
				bat[sprt].readDelayTick = TIMEBETWEENREADS;
				bat[sprt].stateChanges++;
				bat[sprt].statusUpdated = 1;
			} else if (bat[sprt].batteryVoltage < MINVOLTAGE) {
				bat[sprt].batteryState = S_DONE;
				bat[sprt].stateChanges++;
				bat[sprt].statusUpdated = 1;
			}
			break;
		case S_STORECHARGE:
			if (bat[sprt].batteryVoltage >= STORECHARGE) {
				disableCharging(&portsEnabled, sprt);
				disableDischarging(&portsEnabled, sprt);
				rewritePorts = 1;
				bat[sprt].batteryState = S_DONE;
				bat[sprt].stateChanges++;
				bat[sprt].statusUpdated = 1;
				bat[sprt].readDelayTick = 0;
				bat[sprt].readTick = 0;
			} else if (bat[sprt].batteryVoltage < MINVOLTAGE) {
				disableCharging(&portsEnabled, sprt);
				disableDischarging(&portsEnabled, sprt);
				rewritePorts = 1;
				bat[sprt].batteryState = S_DONE;
				bat[sprt].stateChanges++;
				bat[sprt].statusUpdated = 1;
				bat[sprt].readDelayTick = 0;
				bat[sprt].readTick = 0;
			}
			break;
		case S_DONE:
			if (bat[sprt].batteryVoltage < MINVOLTAGE) {
				disableCharging(&portsEnabled, sprt);
				disableDischarging(&portsEnabled, sprt);
				rewritePorts = 1;
				bat[sprt].batteryState = S_DISCONNECTED;
				bat[sprt].stateChanges++;
				bat[sprt].statusUpdated = 1;
				bat[sprt].readDelayTick = 0;
				bat[sprt].readTick = 0;
			}
			break;
		default:
			invalidState++;
		}
	}
	if (rewritePorts) {
		rewritePorts = 0;
		registerWrite(portsEnabled);
	}
}


void formatCapacity(char * printArray, volatile battery_t * bat, uint8_t whichTest) {
	if (bat->batteryState == S_DISCONNECTED || bat->batteryState == S_DONE) {
		snprintf(printArray, PRINTMAX, "%d. ", whichTest + 1);
		printArray += strlen(printArray) - 1;
	}
	if (bat->batteryDischargeTime[whichTest] > 60 * 60) {
		snprintf(printArray, PRINTMAX, "%d.%d mAh %02d:%02dm",
				(uint16_t) bat->batteryCapacity[whichTest],
				((uint16_t) (bat->batteryCapacity[whichTest] * 10) % 10),
				(uint16_t) bat->batteryDischargeTime[whichTest] / 60 / 60,
				(uint16_t) (bat->batteryDischargeTime[whichTest] / 60 % 60));
	} else {
		snprintf(printArray, PRINTMAX, "%d.%d mAh %02d:%02ds",
				(uint16_t) bat->batteryCapacity[whichTest],
				((uint16_t) (bat->batteryCapacity[whichTest] * 10) % 10),
				(uint16_t) bat->batteryDischargeTime[whichTest] / 60,
				(uint16_t) (bat->batteryDischargeTime[whichTest] % 60));
	}
}


// port number, used as iterator in main loop
uint8_t prt;
char printArray[PRINTMAX];
uint8_t finishedIterator;
uint16_t printingTime;
void loop() {
	statisticsDelay.cycle();
	if (chargerState != chargerStatePrev) {
		stopISR();
//		disableADCFreerun();
		chargerStatePrev = chargerState;
		if (chargerState == C_AUTOTEST_S || chargerState == C_AUTOTEST_D) {
			portsEnabled = 0;
			registerWrite(portsEnabled);
			for (prt = 0; prt < NUMBATTERIES; prt++) {
				initBattery(&bat[prt]);
				bat[prt].stateChanges++;
				bat[prt].readTick = READWAIT / 2 * (prt + 1);
			}
//			enableADCFreerun();
		} else if (chargerState == C_CHARGE) {
			for (prt = 0; prt < NUMBATTERIES; prt++) {
				disableCharging(&portsEnabled, prt);
				disableDischarging(&portsEnabled, prt);
				initBattery(&bat[prt]);
				bat[prt].batteryState = S_CHARGING1;
				bat[prt].stateChanges++;
				bat[prt].readTick = READWAIT / 2 * (prt + 1);
			}
			registerWrite(portsEnabled);
		} else if (chargerState == C_DISCHARGE) {
			for (prt = 0; prt < NUMBATTERIES; prt++) {
				initBattery(&bat[prt]);
				bat[prt].batteryState = S_DISCONNECTED;
				bat[prt].stateChanges++;
				bat[prt].readTick = READWAIT / 2 * (prt + 1);
				disableCharging(&portsEnabled, prt);
				disableDischarging(&portsEnabled, prt);
			}
			registerWrite(portsEnabled);
		} else if (chargerState == C_STORE) {
			for (prt = 0; prt < NUMBATTERIES; prt++) {
				disableCharging(&portsEnabled, prt);
				disableDischarging(&portsEnabled, prt);
				initBattery(&bat[prt]);
				bat[prt].batteryState = S_STORECHARGE;
				bat[prt].stateChanges++;
				bat[prt].readTick = READWAIT / 2 * (prt + 1);
			}
			registerWrite(portsEnabled);
		}
		// Update display with current charger state
		tft.fillScreen(ST7735_BLACK);
		tft.setCursor(0,0);
		tft.print(chargerStatesString[chargerState]);
		startISR();
	}
	if (statisticsDelay.hasPassed()) {
		statisticsDelay.reset();
		printingTime++;
		if (printingTime % 20 == 0) {
			Serial.print("Statuses: ");
			for (prt = 0; prt < NUMBATTERIES; prt++) {
				Serial.print(bat[prt].batteryState);
			}
			Serial.println("");
			// Display battery number and voltage
			Serial.print(F("iC: "));
			Serial.print(isrCalled);
			Serial.print(F(" !iS: "));
			Serial.print(invalidState);
			Serial.print(F(" ports: "));
			Serial.print(portsEnabled);
			Serial.print(F(" button:"));
			Serial.print(buttonPressed);
			Serial.print(F(" cs:"));
			Serial.println(chargerState);
	//		Serial.print(F(" Freerun:"));
	//		Serial.println(ADCFreeruns);
			if (isrOverflow) {
				Serial.print(F(" !!overflow:"));
				Serial.println(isrOverflow);
			}
		}
		for (prt = 0; prt < NUMBATTERIES; prt++) {
			if (bat[prt].statusUpdated) {
				bat[prt].statusUpdated = 0;
				if (bat[prt].batteryVoltage > 199) {
					if (bat[prt].batteryState == S_DISCHARGING1 ||
							bat[prt].batteryState == S_DISCHARGING2) {
						if (printingTime % 5 == 0) {
								snprintf(printArray, PRINTMAX, "%d. %d.%02dV",
										prt + 1,
										bat[prt].batteryVoltage / 100,
										bat[prt].batteryVoltage % 100);
								displayVoltage(prt, printArray);
						}
					} else {
						snprintf(printArray, PRINTMAX, "%d. %d.%02dV",
								prt + 1,
								bat[prt].batteryVoltage / 100,
								bat[prt].batteryVoltage % 100);
						displayVoltage(prt, printArray);
					}
				} else {
					snprintf(printArray, PRINTMAX, "%d. %d.%02dV",
							prt + 1, 0, 0);
					displayVoltage(prt, printArray);
				}
			}
			if (printingTime % 20 == 0) {
				printBatteryStatus(&(bat[prt]));
			}
			if (bat[prt].batteryState == S_DISCHARGING1) {
				formatCapacity(printArray, &bat[prt], 0);
				displayCapacity(prt, printArray);
			} else if (bat[prt].batteryState == S_DISCHARGING2) {
				formatCapacity(printArray, &bat[prt], 1);
				displayCapacity(prt, printArray);
			} else if (bat[prt].batteryState == S_CHARGING1 &&
					bat[prt].batteryVoltage > 199) {
				bat[prt].batteryChargeTime[0] += 1;
				snprintf(printArray, PRINTMAX, "%02d:%02d",
						(uint16_t) bat[prt].batteryChargeTime[0] / 60, (uint16_t) (bat[prt].batteryChargeTime[0] % 60));
				displayCapacity(prt, printArray);
			} else if (bat[prt].batteryState == S_CHARGING2 &&
					bat[prt].batteryVoltage > 199) {
				bat[prt].batteryChargeTime[1] += 1;
				formatCapacity(printArray, &bat[prt], 1);
				displayCapacity(prt, printArray);
			} else if (bat[prt].batteryState == S_DONE || 
					((bat[prt].batteryState == S_DISCONNECTED) && (bat[prt].batteryCapacity[0]))) {
				if (finishedIterator == 0 && bat[prt].batteryCapacity[0]) {
					formatCapacity(printArray, &bat[prt], 0);
					displayCapacity(prt, printArray);
				} else if (finishedIterator == 2 && bat[prt].batteryCapacity[1]) {
					formatCapacity(printArray, &bat[prt], 1);
					displayCapacity(prt, printArray);
				}
			}
		}
		if (++finishedIterator == 4) {
			finishedIterator = 0;
		}
	}
}


