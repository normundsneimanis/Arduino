
// Reset solar panel watchdog
// Author: Nick Gammon
// Date: 22 March 2015

// ATMEL ATTINY 25/45/85 / ARDUINO
// Pin 1 is /RESET
//
//                  +-\/-+
// Ain0 (D 5) PB5  1|    |8  Vcc
// Ain3 (D 3) PB3  2|    |7  PB2 (D 2) Ain1 
// Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1
//            GND  4|    |5  PB0 (D 0) pwm0
//                  +----+

/*

 After reset waits for TIME_TO_WAIT minutes, then brings D0 (pin 5) high for long
 enough to activate a MOSFET and reset the other board.
  
 Fuses: Low: E2 High: DD
 
 (Brownout at 2.7V)
 
*/


#include <avr/sleep.h>    // Sleep Modes
#include <avr/power.h>    // Power management
#include <avr/wdt.h>      // Watchdog timer

const byte MOSFET = 0;          // pin 5 
unsigned long counter = 0;
 
const float TIME_TO_WAIT = 30; // minutes
const unsigned long SLEEPS_TO_WAIT = TIME_TO_WAIT * 60.0 / 8.0;  // 8 second sleeps

// watchdog interrupt
ISR (WDT_vect) 
  {
   wdt_disable();  // disable watchdog
  }  // end of WDT_vect

#if defined(__AVR_ATtiny85__)  
  #define watchdogRegister WDTCR
#else
  #define watchdogRegister WDTCSR
#endif
  
void setup()
  {
  wdt_reset();  
  pinMode (MOSFET, OUTPUT);
  ADCSRA = 0;            // turn off ADC
  power_all_disable ();  // power off ADC, Timer 0 and 1, serial interface
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);
  }  // end of setup

void loop()
  {
  counter++;
  
  if (counter >= SLEEPS_TO_WAIT)
    {
    digitalWrite (MOSFET, HIGH);
    delayMicroseconds (10000); 
    digitalWrite (MOSFET, LOW);
    // our job here is done
    sleep_enable ();       // ready to sleep
    sleep_cpu ();          // sleep                
    }
  
  goToSleep ();
  }  // end of loop
  
void goToSleep ()
  {
  noInterrupts ();       // timed sequence coming up
  // pat the dog
  wdt_reset();  
  
  // clear various "reset" flags
  MCUSR = 0;     
  // allow changes, disable reset, clear existing interrupt
  watchdogRegister = bit (WDCE) | bit (WDE) | bit (WDIF);
  // set interrupt mode and an interval (WDE must be changed from 1 to 0 here)
  watchdogRegister = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
  
  sleep_enable();       // ready to sleep
  interrupts();         // interrupts are required now
  sleep_cpu();          // sleep                
  sleep_disable();      // precaution
  }  // end of goToSleep 