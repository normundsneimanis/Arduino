/*
 * TODO
 * on new chip programming - brownout detection @2.7V
 * do not turn on anything when voltage reach 3V
 * do not run for more than 10 hours
 */

/*
 * Port2 -> KTY81-210 temperature sensor -> 2K Ohm resistor -> GND
 *                                       -> A6
 *                                       
 */
#include <BasicSerial.h>
#include "timedelay.h"

void serOut(const char* str) {
   while (*str) TxByte (*str++);
}

char string[40];
timedelay secondDelay(5000);

#define temperaturePort A1 // PB2
#define ledGreen PB0
#define ledRed PB1
#define enableHeater PB3
#define enableFan PB4


#define MAX_TEMPS 44
const int temps[] = {
  980, 1030, 1030 + 53, 1135, 1135 + 56, 1247, 1247 + 60, 1367, 1367 + 64,
  1495, 1495 + 68, 1630, 1630 + 71, 1772, 1772 + 75, 1922, 2000, 2080, 2080 + 83,
  2245, 2245 + 86, 2417, 2417 + 90, 2597, 2597 + 94, 2785, 2785 + 98, 2980,
  2980 + 101, 3182, 3182 + 105, 3392, 3392 + 108, 3607, 3607+105, 3817, 3817+49,
  3915, 3915 + 47, 4008, 4008 + 79, 4166, 4166 + 57, 4280
};

int voltageDivider(float Vin, int R1, int R2) {
  return (int) ((float) (((float) Vin * R2) / (R1 + R2)) / ((float) Vin / 1024.0) + 0.5);
}

/*
 * Temperature sensor KTY81/210 used in voltage divider
 * +5v -- temperature sensor -- analog port -- 2kOhm resistor -- ground
 * TODO: Instead of using loops, use simple math to calculate the place
 */
int toCelsius(float voltage, int reading) {
  int voltage1, voltage2;
  int tempIdx = 0;
  while (1) {
    voltage1 = voltageDivider(voltage, temps[tempIdx], 2200);
    voltage2 = voltageDivider(voltage, temps[tempIdx+1], 2200);
    if (reading >= voltage2 && reading <= voltage1) {
      break;
    }
    // handle error condition
    if (tempIdx == MAX_TEMPS) {
      return -160;
    }
    tempIdx++;
  }
  int startTemp = -55;
  for (int i = 0; i < tempIdx; i++) {
    startTemp += 5;
  }
  int range = voltage1 - voltage2;
  range /= 5;
  voltage2 = voltage1;
  voltage1 -= range;
  for (int i = 0; i < 5; i++) {
    if (reading >= voltage1 && reading <= voltage2) {
      startTemp++;
      break;
    }
    startTemp++;
    voltage1 -= range;
    voltage2 -= range;
  }

  return startTemp;
}

void startISR() {
  TCCR1 = 0; // Stop timer so we can modify it
  TCNT1 = 0;
  GTCCR = _BV(PSR1);          //reset the prescaler
  OCR1C = 98; // Set CTC compare value to 10Hz at 8MHz AVR clock, with a prescaler of 8192
  TCCR1 |= _BV(CTC1) | _BV(CS11) | _BV(CS12) | _BV(CS13); // Set up timer at Fcpu/8192
  TIMSK |= _BV(OCIE1A); // Enable CTC interrupt
  sei(); //  Enable global interrupts
}

volatile uint32_t interruptCounter = 0;
volatile uint32_t secondsCounter = 0;
ISR(TIMER1_COMPA_vect) {
  if (interruptCounter++ % 10 == 0) {
    secondsCounter++;
    interruptCounter = 1;
  }
}

// internalRefCalibration
// internalRef = 1.1 * Vcc1 (per voltmeter) / Vcc2 (per readVcc() function)
float internalRef = 1.1 * 4940 / 5516;
int32_t internalRefConstant = internalRef * 1023 * 1000;
uint32_t readInternalVoltage() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = internalRefConstant / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

uint8_t initialTemp;
uint32_t oldCounter;
int temperatureReading;
void setup() {
  // read initial temperature
  // set up ouput ports, remaining are left as inputs
  DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB3) | (1 << DDB4);
  // PORTB |= (1 << PORTB6); // PB6 high
  // PORTB &= ~(1 << PORTB6); // PB6 low
  temperatureReading = analogRead(temperaturePort);
  initialTemp = toCelsius(readInternalVoltage() / 1000, temperatureReading) - 5;
  // enable heater
  
  startISR();

}

uint8_t fanEnabled;
void loop() {
  temperatureReading = analogRead(temperaturePort);
  temperatureReading = toCelsius(readInternalVoltage() / 1000, temperatureReading) - 3;
  if (!fanEnabled && (temperatureReading >= initialTemp + 10 || temperatureReading >= 35 || secondsCounter >= 60*2)) {
    // enable Fan
    PORTB |= (1 << PORTB4);
    // Disable heater
    PORTB &= ~(1 << PORTB3);
    fanEnabled = 1;
    cli();
    secondsCounter = 0;
    sei();
    oldCounter = 0;
  }
  if (fanEnabled && secondsCounter >= 60*2) {
    cli();
    secondsCounter = 0;
    sei();
    oldCounter = 0;
    fanEnabled = 0;
    // disableFan
    PORTB &= ~(1 << PORTB4);
    // enable heater
    PORTB |= (1 << PORTB3);
  }
  
  if (secondsCounter >= oldCounter + 3) {
    oldCounter = secondsCounter;
    if (temperatureReading >= initialTemp + 10) {
      // blink green led once
      PORTB |= (1 << PORTB0);
      delay(50);
      PORTB &= ~(1 << PORTB0);
      delay(50);
      PORTB |= (1 << PORTB0);
      delay(50);
      PORTB &= ~(1 << PORTB0);
    } else if (temperatureReading >= 35) {
      PORTB |= (1 << PORTB0);
      delay(50);
      PORTB &= ~(1 << PORTB0);
      delay(50);
      PORTB |= (1 << PORTB0);
      delay(50);
      PORTB &= ~(1 << PORTB0);
      delay(50);
      PORTB |= (1 << PORTB0);
      delay(50);
      PORTB &= ~(1 << PORTB0);
    } else {
      // blink yellow led if heater is enabled
      if (!fanEnabled) {
        PORTB |= (1 << PORTB1);
        delay(50);
        PORTB &= ~(1 << PORTB1);
      } else { // blink green led when fan is enabled
        PORTB |= (1 << PORTB0);
        delay(50);
        PORTB &= ~(1 << PORTB0);
      }
    }
    // lit red led for half a second if sensor is not working
    if (analogRead(temperaturePort) == 0) {
      PORTB |= (1 << PORTB1);
      delay(500);
      PORTB &= ~(1 << PORTB1);
    }
    
//    temperatureReading = analogRead(temperaturePort);
//    // Convert temperature reading to degrees celsius
//    sprintf(string, "%d ", temperatureReading);
//    serOut("RawReading: ");
//    serOut(string);
//    sprintf(string, "%d ", (int) readInternalVoltage());
//    serOut(" voltageRaw: ");
//    serOut(string);
//    sprintf(string, "%.2f ", (((double) readInternalVoltage())/1000));
//    serOut(" voltage: ");
//    serOut(string);
//    temperatureReading = toCelsius(readInternalVoltage() / 1000, temperatureReading) - 3;
//    sprintf(string, " %d\n", temperatureReading);
//    serOut(string);
  }
}
