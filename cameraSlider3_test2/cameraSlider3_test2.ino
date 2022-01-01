#include "hallSensorInput.h"
#include "timedelay.h"
#include "rotaryEncoder.h"

/* Arduino Pro Mini
#define cs_sd   10
#define cs_lcd   9
#define dc       8
#define rst      7
*/

/* Slider Controller */
#define cs_lcd   9
#define dc       10
#define rst      7

/* 
 *  Camera photo requests
 *    focus  10  6
 *    pic    16 12
 */
#define cameraTriggerFocus 6
#define cameraTriggerPic 12


/* Camera photo trigger 
 *  2 (4) 
 *  9 (5) 
 */
#define triggerFocus 4
#define triggerPic 5

/* Slider direction requests 
 *  left 12  (8)
 *  right 23 (A0)

*/
#define directionSwitchLeft 8
#define directionSwitchRight A0

/* Analog input pins
 *  sens1 27 (A4)
 *  sens2 28 (A5)
*/
#define hallSensorLeft A4
#define hallSensorRight A5

/* rotary encoder 
 *  button - 26  (A3)
 *  right - 24   (A1)
 *  left - 32    (2)

*/
#define encoderPinLeft 2
#define encoderPinRight A1
#define encoderButtonPin A3

/* 
 *  Voltage sensor
 *  19 (A6)
 */
#define batterySensor A6

timedelay StatisticsDelay(330); // How frequently to print statistics
timedelay triggerDelay(2000); // Testing camera trigger delay
rotaryEncoder encoder(encoderPinLeft, encoderPinRight, encoderButtonPin);
/*
 * SD card currently is not working
 * 
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
#include <SD.h>

#define BUFFPIXEL 20

// built in SPI interface
Adafruit_ST7735 tft = Adafruit_ST7735(cs_lcd, dc, rst);

// Software SPI interface
//#define MOSI 11
//#define SCK 13
//Adafruit_ST7735 tft = Adafruit_ST7735(cs_lcd, dc, MOSI, SCK, rst);

// ISR to aggregate rotary encoder movement
void rotaryEncoderISR() {
  if (digitalRead(encoderPinLeft) == digitalRead(encoderPinRight)) {
    encoder.stepUp();
  } else {
    encoder.stepDown();
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(cameraTriggerFocus, INPUT);
  pinMode(cameraTriggerPic, INPUT);
  pinMode(triggerFocus, OUTPUT);
  pinMode(triggerPic, OUTPUT);

  pinMode(directionSwitchLeft, INPUT);
  digitalWrite(directionSwitchLeft, HIGH);
  pinMode(directionSwitchRight, INPUT);
  digitalWrite(directionSwitchRight, HIGH);

  pinMode(hallSensorLeft, INPUT);
  pinMode(hallSensorRight, INPUT);

  pinMode(batterySensor, INPUT);
  
 tft.initR(INITR_BLACKTAB);
 tft.fillScreen(ST7735_BLACK);
 tft.setRotation(1); // 0 - Portrait, 1 - Lanscape
 tft.setTextWrap(false);

  tft.fillScreen (ST7735_BLACK);
  tft.fillRect (0, 0, 160, 16, ST7735_GREEN);
  tft.fillRect (0, 24, 160, 16 ,ST7735_RED);

 delay(2000);

  tft.fillScreen (ST7735_BLACK);
  tft.fillRect (0, 0, 160, 16, ST7735_GREEN);
  tft.fillRect (0, 24, 160, 16 ,ST7735_BLUE);
  testdrawtext ("         111111111222123456789012345678901\nScreen is 21 x 20\ncharacters in size.\n", ST7735_WHITE);

  tft.setCursor (1, 48);
  tft.println ("\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz~ right up to ASCII 255\n\nThe quick brown fox  jumped over the lazy dog.\n\n");

  tft.setTextColor(ST7735_RED);
  tft.setCursor (1, 128-18);
  tft.println ("Countdown:");

  tft.setTextColor (ST7735_BLACK);
  tft.setTextSize (2);
  for (int i=6; i>=0; i--) {
     tft.setCursor (2, 128-18);
     tft.fillRect (0, 128-18, 160, 18, ST7735_RED);
     tft.print (i);
     tft.print (" seconds");
     delay(1000);
  }

 tft.setTextSize(5);
 tft.setTextColor(ST7735_WHITE, ST7735_BLACK);


  encoder.init();
  attachInterrupt(digitalPinToInterrupt(encoderPinLeft), rotaryEncoderISR, CHANGE);
  

//  delay(10000);
//  tft.sleep();
//  delay(10000);
//  tft.wake();

// if (!SD.begin(cs_sd)) {
//   Serial.println("failed!");
//   return;
// } else {
//  bmpDraw("test.bmp", 0, 0);
//  delay(10000);
// }
}

int printIterator = 0;
int cameraTriggerState = 0;

void loop() {
  StatisticsDelay.cycle();
  triggerDelay.cycle();
  if (StatisticsDelay.hasPassed()) {
    StatisticsDelay.reset();
    Serial.print("CamFoc: ");
    Serial.print(digitalRead(cameraTriggerFocus));
    Serial.print(" CamPic: ");
    Serial.print(digitalRead(cameraTriggerPic));
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
    Serial.println(map(analogRead(batterySensor), 0, 1023, 0, 20));
//    Serial.println(analogRead(batterySensor));
  }
  if (triggerDelay.hasPassed()) {
    triggerDelay.reset();
    if (cameraTriggerState == 3) {
//      Serial.println("Entering state 3. Resetting");
      cameraTriggerState = 0;
    }
    
    if (cameraTriggerState == 2) {
      cameraTriggerState++;
//      Serial.println("Entering state 2. All lights off");
      digitalWrite(triggerFocus, LOW);
      digitalWrite(triggerPic, LOW);
    }
    
    if (cameraTriggerState == 1) {
//      Serial.println("Entering state 1. Pic");
      cameraTriggerState++;
      digitalWrite(triggerPic, HIGH);
    }
    
    if (cameraTriggerState == 0) {
//      Serial.println("Entering state 0. Focus");
      cameraTriggerState++;
      digitalWrite(triggerFocus, HIGH);
    }
  }
  
  tft.setCursor(40, 50);
  tft.print(printIterator++);
  if (printIterator == 100) {
    printIterator = 0;
  }
}

void testdrawtext(char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void bmpDraw(char *filename, uint8_t x, uint8_t y) {

 File     bmpFile;
 int      bmpWidth, bmpHeight;   // W+H in pixels
 uint8_t  bmpDepth;              // Bit depth (currently must be 24)
 uint32_t bmpImageoffset;        // Start of image data in file
 uint32_t rowSize;               // Not always = bmpWidth; may have padding
 uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
 uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
 boolean  goodBmp = false;       // Set to true on valid header parse
 boolean  flip    = true;        // BMP is stored bottom-to-top
 int      w, h, row, col;
 uint8_t  r, g, b;
 uint32_t pos = 0, startTime = millis();

 if((x >= tft.width()) || (y >= tft.height())) return;

 Serial.println();
 Serial.print("Loading image '");
 Serial.print(filename);
 Serial.println('\'');

 // Open requested file on SD card
 if ((bmpFile = SD.open(filename)) == NULL) {
   Serial.print("File not found");
   return;
 }

 // Parse BMP header
 if(read16(bmpFile) == 0x4D42) { // BMP signature
   Serial.print("File size: "); Serial.println(read32(bmpFile));
   (void)read32(bmpFile); // Read & ignore creator bytes
   bmpImageoffset = read32(bmpFile); // Start of image data
   Serial.print("Image Offset: "); Serial.println(bmpImageoffset, DEC);
   // Read DIB header
   Serial.print("Header size: "); Serial.println(read32(bmpFile));
   bmpWidth  = read32(bmpFile);
   bmpHeight = read32(bmpFile);
   if(read16(bmpFile) == 1) { // # planes -- must be '1'
     bmpDepth = read16(bmpFile); // bits per pixel
     Serial.print("Bit Depth: "); Serial.println(bmpDepth);
     if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

       goodBmp = true; // Supported BMP format -- proceed!
       Serial.print("Image size: ");
       Serial.print(bmpWidth);
       Serial.print('x');
       Serial.println(bmpHeight);

       // BMP rows are padded (if needed) to 4-byte boundary
       rowSize = (bmpWidth * 3 + 3) & ~3;

       // If bmpHeight is negative, image is in top-down order.
       // This is not canon but has been observed in the wild.
       if(bmpHeight < 0) {
         bmpHeight = -bmpHeight;
         flip      = false;
       }

       // Crop area to be loaded
       w = bmpWidth;
       h = bmpHeight;
       if((x+w-1) >= tft.width())  w = tft.width()  - x;
       if((y+h-1) >= tft.height()) h = tft.height() - y;

       // Set TFT address window to clipped image bounds
       tft.setAddrWindow(x, y, x+w-1, y+h-1);

       for (row=0; row<h; row++) { // For each scanline...

         // Seek to start of scan line.  It might seem labor-
         // intensive to be doing this on every line, but this
         // method covers a lot of gritty details like cropping
         // and scanline padding.  Also, the seek only takes
         // place if the file position actually needs to change
         // (avoids a lot of cluster math in SD library).
         if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
           pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
         else     // Bitmap is stored top-to-bottom
           pos = bmpImageoffset + row * rowSize;
         if(bmpFile.position() != pos) { // Need seek?
           bmpFile.seek(pos);
           buffidx = sizeof(sdbuffer); // Force buffer reload
         }

         for (col=0; col<w; col++) { // For each pixel...
           // Time to read more pixel data?
           if (buffidx >= sizeof(sdbuffer)) { // Indeed
             bmpFile.read(sdbuffer, sizeof(sdbuffer));
             buffidx = 0; // Set index to beginning
           }

           // Convert pixel from BMP to TFT format, push to display
           b = sdbuffer[buffidx++];
           g = sdbuffer[buffidx++];
           r = sdbuffer[buffidx++];
           tft.pushColor(tft.Color565(r,g,b));
         } // end pixel
       } // end scanline
       Serial.print("Loaded in ");
       Serial.print(millis() - startTime);
       Serial.println(" ms");
     } // end goodBmp
   }
 }

 bmpFile.close();
 if(!goodBmp) Serial.println("BMP format not recognized.");
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f) {
 uint16_t result;
 ((uint8_t *)&result)[0] = f.read(); // LSB
 ((uint8_t *)&result)[1] = f.read(); // MSB
 return result;
}

uint32_t read32(File f) {
 uint32_t result;
 ((uint8_t *)&result)[0] = f.read(); // LSB
 ((uint8_t *)&result)[1] = f.read();
 ((uint8_t *)&result)[2] = f.read();
 ((uint8_t *)&result)[3] = f.read(); // MSB
 return result;
}
