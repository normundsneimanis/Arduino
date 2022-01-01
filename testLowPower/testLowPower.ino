#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

// View fuses in Linux?
// sudo avrdude -P /dev/ttyACM0 -b 19200 -c arduino -p m328p -v
// Write fuses and hex file
// avrdude -c usbtiny -p atmega328p -q -U flash:w:filename.hex -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0x07:m


#define LED 8

// watchdog interrupt
ISR (WDT_vect) 
  {
  wdt_disable();  // disable watchdog
  }  // end of WDT_vect



void setup() {
  // put your setup code here, to run once:
  // slow clock down to 4 MHz
  
  clock_prescale_set (clock_div_2);

  // set pins to OUTPUT and LOW  
  for (byte i = 0; i <= A5; i++)
    {
    // skip radio pins
    if (i >= 9 && i <= 13)
      continue;
    pinMode (i, OUTPUT);    
    digitalWrite (i, LOW);  
    }  // end of for loop

  ADCSRA = 0;  // disable ADC
  power_all_disable ();   // turn off all modules
}

unsigned int counter;

void loop() {
  // put your main code here, to run repeatedly:
  // every 64 seconds send a reading
  if ((++counter & 7) == 0)
    {
        power_all_enable();
    digitalWrite (LED, HIGH);
    delay(1000);
    digitalWrite (LED, LOW);
    for (byte i = 9; i <= 13; i++)
      {
      pinMode (i, OUTPUT);    
      digitalWrite (i, LOW); 
      }  // end of for loop
    ADCSRA = 0;  // disable ADC
    power_all_disable();
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
