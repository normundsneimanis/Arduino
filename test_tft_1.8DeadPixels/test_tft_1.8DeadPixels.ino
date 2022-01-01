/* Arduino Pro Mini
#define cs_sd   10
#define cs_lcd   9
#define dc       8
#define rst      7
#define switchPin 3
*/

/* Slider Controller */
#define cs_lcd   9
#define dc       10
#define rst      7
#define switchPin 5

/*
 * Connections:
 * Arduino -> Display (port)
 * 7 -> R 10K -> RST 1
 * 8 -> R 10K -> D/C 3
 * 9 -> R 10K -> CS 2
 * 11 -> R 10K -> DIN 4
 * 13 -> R 10K -> CLK 5
 */


#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#define BUFFPIXEL 20

// built in SPI interface
Adafruit_ST7735 tft = Adafruit_ST7735(cs_lcd, dc, rst);

void setup() {
 Serial.begin(115200);

 pinMode(switchPin, INPUT);
 digitalWrite(switchPin, HIGH);

 tft.initR(INITR_BLACKTAB);
 tft.fillScreen(ST7735_BLACK);
 tft.setRotation(1); // 0 - Portrait, 1 - Lanscape
 tft.setTextWrap(false);
}

int iteratorPrev = 0;
int iterator = 0;
void loop() {
  if (iterator != iteratorPrev) {
    iteratorPrev = iterator;
    switch (iterator) {
      case 0:
        tft.fillScreen(ST7735_BLACK);
        break;
      case 1:
        tft.fillScreen(ST7735_WHITE);
        break;
      case 2:
        tft.fillScreen(ST7735_RED);
        break;
      case 3:
        tft.fillScreen(ST7735_GREEN);
        break;
      case 4:
        tft.fillScreen(ST7735_BLUE);
        break;
    }
    delay(2000);
  }
  if (!digitalRead(switchPin)) {
    iterator++;
    if (iterator > 4) {
      iterator = 0;
    }
  }
  delay(500);
}
