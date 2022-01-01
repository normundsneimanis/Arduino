#include "U8glib.h"
#include "timedelay.h"
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);  // I2C / TWI 

#define displayEnablePort A0
// LED ports 10-12 on uC. Red, white and green leds
#define ledRed 6
#define led1 7
#define led2 8
#define enableFets 5
#define potPin A2
#define weldingRequestPort A1

timedelay secondDelay(5000);

// Used for software button debouncing
volatile uint8_t buttonRead;
// Interrupt on keypress, port 24, PC1, PCINT9
// Count how many times button press interrupt was called
volatile uint32_t buttonCalls;
ISR (PCINT1_vect) {
  buttonRead = 10;
  // disable external interrupt on this pin
  PCMSK1 &= ~_BV(PCINT9);
  buttonCalls++;
}

volatile uint8_t weldingTime;
volatile uint16_t weldingRequests;
volatile uint16_t weldingRequestFails;
volatile uint16_t weldingDelay;
volatile uint8_t weldPass;
uint8_t weldingTimeRequestedPrev;
uint8_t weldingTimeRequested = 1; // welding time in milliseconds
ISR(TIMER1_COMPA_vect) {
  if (buttonRead > 0) {
    if (--buttonRead == 0) {
      // re-read button and if button is ON, trigger welding
      if (!(PINC & _BV(PC1))) {
        weldingTime = weldingTimeRequested / 8;
        if (!weldingTime) {
          weldingTime = 1;
        }
        weldPass = 1;
        // set welding port and status led ports to ON
        PORTD |= (_BV(PORTD5) | _BV(PORTD6));
        weldingRequests++;
        weldingDelay = 1000;
      } else {
		// Count how many requests were denied because of debouncing
        weldingRequestFails++;
        weldingDelay = 200;
      }
    }
  }


  
  if (weldingTime) {
    if (weldingTime > 1) {
      weldingTime--;
    } else if (weldingTime == 1) {
      // switch off welding and status led ports
      PORTD &= ~(_BV(PORTD5) | _BV(PORTD6));
      weldingTime = 0;
    }
  } else if (weldPass) {
    weldPass = 0;
    weldingTime = weldingTimeRequested;
    PORTD |= (_BV(PORTD5) | _BV(PORTD6));
  }

  if (weldingDelay) {
    if (--weldingDelay == 0) {
      PCMSK1 |= _BV(PCINT9); /* Enable PCINT9 interrupt */
    }
  }
}
// we need timer with 1 millisecond resolution, it will be used for external 
// interrupt read and for precise pulse triggering
void startISR() {
  TCCR1A = 0;
  TCCR1B = 0; // Stop timer so we can modify it
  TCCR1B |= _BV(WGM12); // Configure timer 1 for CTC mode
  // Timer for 16MHz left for reference as board was designed for 16MHz MCU
  //OCR1A   = 250 - 1; // Set CTC compare value to 1000Hz at 16MHz AVR clock, with a prescaler of 64
  OCR1A   = 125 - 1; // Set CTC compare value to 1000Hz at 8MHz AVR clock, with a prescaler of 64
  TIMSK1 |= _BV(OCIE1A); // Enable CTC interrupt
  TCCR1B |= (_BV(CS11) | _BV(CS10)); // Set up timer at Fcpu/64
  sei(); //  Enable global interrupts
}

#define PRINTSTRMAX 10
char printStr[PRINTSTRMAX];
void drawText(int weldingTimeRequested) {
  // graphic commands to redraw the complete screen should be placed here  
  // Font used originally
  // u8g.setFont(u8g_font_unifont);
  u8g.setFont(u8g_font_fur49n);
  snprintf(printStr, PRINTSTRMAX, "%d", weldingTimeRequested);
  u8g.drawStr(40, 39, printStr);
  //u8g.drawStr(0, 30, "Quick brown fox");
  //u8g.drawStr(0, 40, "jumped over the");
  //u8g.drawStr(0, 50, "lazy dog!");
  //u8g.drawStr(0, 60, "Some more text");
  
}

void setup() {
  PCMSK1 |= _BV(PCINT9); /* Enable PCINT9 */
  PCICR |= _BV(PCIE1); /* Activate interrupt on enabled PCINT14-8 */
  
  pinMode(enableFets, OUTPUT);
  digitalWrite(enableFets, LOW);
  pinMode(weldingRequestPort, INPUT);
  digitalWrite(weldingRequestPort, HIGH);
  pinMode(displayEnablePort, OUTPUT);
  digitalWrite(displayEnablePort, HIGH);
  pinMode(ledRed, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(potPin, INPUT);
  Serial.begin(9600);
  digitalWrite(ledRed, HIGH);
  delay(500);
  digitalWrite(ledRed, LOW);
  digitalWrite(led1, HIGH);
  delay(500);
  digitalWrite(led1, LOW);
  digitalWrite(led2, HIGH);
  delay(500);
  digitalWrite(led2, LOW);
  startISR();
  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
  u8g.setFont(u8g_font_unifont);
  u8g.firstPage();
  do {
    drawText();
  } while( u8g.nextPage() );
  delay(3000);
}

void loop() {
  readingDelay.cycle()

  weldingTimeRequested = map(analogRead(potPin), 0, 1023, 1, 20);
  if (weldingTimeRequested != weldingTimeRequestedPrev) {
    weldingTimeRequestedPrev = weldingTimeRequested;
    do {
      drawText(weldingTimeRequested);
    } while(u8g.nextPage());
  } else {
    delay(50);
  }

  if (readingDelay.hasPassed()) {
    readingDelay.reset();
    digitalWrite(led1, HIGH);
    Serial.print("PC1: ");
    Serial.print(PINC);
    Serial.print(" Welding time: ");
    Serial.print(weldingTimeRequested);
    Serial.print(" buttonCalls: ");
    Serial.print(buttonCalls);
    Serial.print(" weldingRequests: ");
    Serial.print(weldingRequests);
    Serial.print(" weldingRequestFails: ");
    Serial.println(weldingRequestFails);
    digitalWrite(led1, LOW);
  }
}


