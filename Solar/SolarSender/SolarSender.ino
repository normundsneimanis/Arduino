// Program to send its own voltage reading by NRF24L01 to another processor.
// Author: Nick Gammon
// Date: 15th March 2015

// NB: Compile for Lilypad/328 because of the 8 MHz clock
//     Note that we divide clock by 2 so we can run at 1.8V
//     Thus, delays will be twice as long.
//  Fuses:  Low: E2  High: DF  Ext: 07
//      That sets 8 MHz internal clock, and brownout detection disabled
//  Linux: avrdude -P /dev/ttyACM0 -c arduino -p atmega328p -q
//	  sudo avrdude -P /dev/ttyACM0 -c arduino -p atmega328p -q -U lfuse:w:0xE2:m -U hfuse:w:0xDF:m -U efuse:w:0xFF:m
// read flash: sudo avrdude -P /dev/ttyACM0 -c arduino -p atmega328p -q -U flash:r:flash.bin:r
//		buspirate: sudo avrdude -P /dev/ttyUSB4 -c buspirate -p atmega328p -q -U flash:r:flash.bin:r
// write flash: sudo avrdude -P /dev/ttyUSB4 -c buspirate -p atmega328p -q -U flash:w:testLowPower.ino.hex:r
// Fuse calculator: http://www.engbedded.com/fusecalc/
//
// sudo avrdude -P /dev/ttyUSB4 -c buspirate -p atmega328p -q -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0x07:m
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include "solarPacket.h"

// Change 0 to temperature differrence you see when measure new chip
#define tempCoefficient 5 * 1.4

#define humidityPort A0

#define humidityPowerPort A1
const byte LED = 8;
const byte CHIP_ENABLE = 9;
const byte CHIP_SELECT = 10;
//unsigned long timeStamps[10];
//unsigned int voltageReading;

const float InternalReferenceVoltage = 1.05; // as measured

// watchdog interrupt
ISR (WDT_vect) {
  wdt_disable();  // disable watchdog
}

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xc897f8d075LL, 0x5feddd192fLL };
unsigned int counter; // wake up counter
solarPacket packet;
unsigned int rawTemperature;

void setup() {
  // slow clock down to 4 MHz

  clock_prescale_set(clock_div_2);

  // set pins to OUTPUT and LOW
  for (byte i = 0; i <= A5; i++) {
    // skip radio pins
    if (i >= 9 && i <= 13)
      continue;
    pinMode (i, OUTPUT);
    digitalWrite (i, LOW);
  }  // end of for loop

  ADCSRA = 0;  // disable ADC
  Serial.begin(9600);
  Serial.println("Starting");
  power_all_disable();   // turn off all modules

}  // end of setup

void sendVoltage() {
  // Set up nRF24L01 radio on SPI bus plus pins 9 & 10
  RF24 radio(CHIP_ENABLE, CHIP_SELECT);

  power_all_enable();
  // timeStamps[0] = millis();
  digitalWrite(SS, HIGH);
  SPI.begin();
  digitalWrite(CHIP_ENABLE, LOW);
  digitalWrite(CHIP_SELECT, HIGH);

  //
  // Setup and configure rf radio
  //
  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15, 15);

  // optionally, reduce the payload size.  seems to improve reliability
  radio.setPayloadSize(8);

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);

  radio.startListening();
  delay(10);
  radio.stopListening();

  // Take the voltage, and send it.  This will block until complete
  ADCSRA =  bit (ADEN);   // turn ADC on
  ADCSRA |= bit (ADPS0) |  bit (ADPS1) | bit (ADPS2);  // Prescaler of 128
  ADMUX = bit (REFS0) | bit (MUX3) | bit (MUX2) | bit (MUX1);

  bitSet(ADCSRA, ADSC);  // start a conversion
  while (bit_is_set(ADCSRA, ADSC))
  {}

  // and again, <sigh>, because first reading after enabling ADC could be inaccurate
  bitSet (ADCSRA, ADSC);  // start a conversion
  while (bit_is_set(ADCSRA, ADSC))
  {}

  packet.voltageReading = ADC;
  //  voltageReading = ADC;

  //  timeStamps[1] = millis();
  pinMode(humidityPort, INPUT);
  pinMode(humidityPowerPort, OUTPUT);
  digitalWrite(humidityPowerPort, HIGH);

  //	ADMUX = 0x00; // ADC input channel set to PC0 (port 23)
  //	bitSet(ADCSRA, ADSC);  // start a conversion
  packet.voltage = (int) ((InternalReferenceVoltage / float ((float) packet.voltageReading + 0.5) * 1024.0) * 100);
  //	while (bit_is_set(ADCSRA, ADSC))
  //		{}
  //
  //	// Reading register "ADCW" takes care of how to read ADCL and ADCH.
  //  packet.humidity = ADCW;
  packet.humidity = analogRead(humidityPort);
  packet.humidity = analogRead(humidityPort);
  pinMode(humidityPort, OUTPUT);
  digitalWrite(humidityPort, LOW);
  pinMode(humidityPowerPort, OUTPUT);
  digitalWrite(humidityPowerPort, LOW);
  //  timeStamps[2] = millis();

  // Read internal temperature sensor
  ADMUX = (bit(REFS1) | bit(REFS0) | bit(MUX3));
  bitSet(ADCSRA, ADSC);  // start a conversion
  while (bit_is_set(ADCSRA, ADSC))
  {}

  rawTemperature = ADCW;
  packet.temperature = (int) ((rawTemperature - 324.31 + tempCoefficient ) / 1.22 * 100);

  radio.write(&packet, sizeof packet);

  radio.startListening();
  radio.powerDown();

  SPI.end();
  //  timeStamps[3] = millis();

  // set pins to OUTPUT and LOW
  for (byte i = 9; i <= 13; i++) {
    pinMode(i, OUTPUT);
    digitalWrite (i, LOW);
  }  // end of for loop
  ADCSRA = 0;  // disable ADC
  // timeStamps[4] = millis();

  // Serial.print("Initialization and reading: ");
  // Serial.print(timeStamps[1] - timeStamps[0]);
  // Serial.print(" Humidity: ");
  // Serial.print(timeStamps[2] - timeStamps[1]);
  // Serial.print(" Sending data: ");
  // Serial.print(timeStamps[3] - timeStamps[2]);
  // Serial.print(" Shutting down: ");
  // Serial.println(timeStamps[4] - timeStamps[3]);
  power_all_disable();

}  // end of sendVoltage

void loop() {
  // every 64 seconds send a reading
  //	if ((++counter & 7) == 0) { // original
  if ((++counter & 2) == 0) { // testing
    //		digitalWrite(LED, HIGH);
    //    delay(5);
    sendVoltage();
    Serial.println("Sent packet");
    //        digitalWrite(LED, LOW);
  } // end of 64 seconds being up

  // clear various "reset" flags
  MCUSR = 0;
  // allow changes, disable reset
  WDTCSR = bit (WDCE) | bit (WDE);
  // set interrupt mode and an interval
  WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
  wdt_reset();  // pat the dog

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();
  sleep_disable();
}
