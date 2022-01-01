#include "hallSensorInput.h"
#include "timedelay.h"
#include "rotaryEncoder.h"
//#include "menu.h"
//#include "images.h"

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <Fonts/FreeSerif9pt7b.h>
#include <SPI.h>

#include <avr/io.h> 
#include <avr/interrupt.h> 

/* Arduino Pro Mini
#define cs_sd   10
#define cs_lcd   9
#define dc       8
#define rst      7
*/

/* Slider Controller Atmega644 */
// RST PB2  2
// CS  PB4  4
// DC  PB3  3
#define cs_lcd   4
#define dc       3
#define rst      2

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

timedelay StatisticsDelay(330); // How frequently to print statistics
timedelay triggerDelay(2000); // Testing camera trigger delay
rotaryEncoder encoder(encoderPinLeft, encoderPinRight, encoderButtonPin);
int cameraTriggerState = 0;
// built in SPI interface
Adafruit_ST7735 tft = Adafruit_ST7735(cs_lcd, dc, rst);

// ISR to aggregate rotary encoder movement
ISR (PCINT2_vect) {
  if (digitalRead(encoderPinLeft) == digitalRead(encoderPinRight)) {
    encoder.stepUp();
  } else {
    encoder.stepDown();
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting");

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

  PCMSK2 |= (1 << PCINT17); /* Enable PCINT17 */
  PCICR |= (1 << PCIE2); /* Activate interrupt on enabled PCINT23-16 */
  sei();                    // turn on interrupts

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1); // 0 - Portrait, 1 - Landscape

  tft.fillScreen (ST7735_BLACK);
  tft.fillRect (0, 0, 160, 16, ST7735_GREEN);
  tft.fillRect (0, 24, 160, 16 ,ST7735_BLUE);
  testdrawtext ("         111111111222123456789012345678901\nScreen is 21 x 20\ncharacters in size.\n", ST7735_WHITE);

  tft.setCursor (1, 48);
  tft.println ("\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz~ right up to ASCII 255\n\nThe quick brown fox  jumped over the lazy dog.\n\n");
}

/* testing slider hardware functionality */
void loop() {
    StatisticsDelay.cycle();
    triggerDelay.cycle();
    if (StatisticsDelay.hasPassed()) {
    StatisticsDelay.reset();
    Serial.print("CamFoc: ");
    Serial.print(!digitalRead(cameraTriggerFocus));
    Serial.print(" CamPic: ");
    Serial.print(!digitalRead(cameraTriggerPic));
    Serial.print(" Enc Press: ");
    Serial.print(encoder.buttonPressed());
    Serial.print(" Enc count: ");
    Serial.print(encoder.getCurrentPosition());
    Serial.print(" Left: ");
    Serial.print(!digitalRead(directionSwitchLeft));
    Serial.print(" Right: ");
    Serial.print(!digitalRead(directionSwitchRight));
    Serial.print(" End1: ");
    Serial.print(analogRead(hallSensorLeft));
    Serial.print(" End2: ");
    Serial.print(analogRead(hallSensorRight));
    Serial.print(" Voltage: ");
    Serial.print(analogRead(batterySensor));
    Serial.print(" - ");
    Serial.println(map(analogRead(batterySensor), 0, 1023, 0, 2650) / 100.0);
    tft.setCursor(2, 128-18);
    tft.fillRect(0, 128-18, 160, 18, ST7735_RED);
    tft.print(map(analogRead(batterySensor), 0, 1023, 0, 2650) / 100.0);
    tft.print(" volts");
  }
  if (triggerDelay.hasPassed()) {
    triggerDelay.reset();
    if (cameraTriggerState == 3) {
      Serial.println("Entering state 3. Resetting");
      cameraTriggerState = 0;
    }
    
    if (cameraTriggerState == 2) {
      cameraTriggerState++;
      Serial.println("Entering state 2. All lights off");
      digitalWrite(triggerFocus, LOW);
      digitalWrite(triggerPic, LOW);
    }
    
    if (cameraTriggerState == 1) {
      Serial.println("Entering state 1. Pic");
      cameraTriggerState++;
      digitalWrite(triggerPic, HIGH);
    }
    
    if (cameraTriggerState == 0) {
      Serial.println("Entering state 0. Focus");
      cameraTriggerState++;
      digitalWrite(triggerFocus, HIGH);
    }
  }
}

void testdrawtext(char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

