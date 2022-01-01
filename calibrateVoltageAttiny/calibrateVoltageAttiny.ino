#include <BasicSerial.h>
// http://electronics.stackexchange.com/questions/4531/serial-newbie-why-cant-i-just-hook-the-wires-up
// http://www.ernstc.dk/arduino/tinycom.html

 void serOut(const char* str) {
   while (*str) TxByte (*str++);
}

void setup() {
  // put your setup code here, to run once:
  serOut("Text string\n"); // (readInternalVoltage());
}

char string[20];

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

  // when calibrating, use 1125300L instead of internalRefConstant
  result = internalRefConstant / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  sprintf(string, "%d\n", readInternalVoltage());
  serOut(string);
}
