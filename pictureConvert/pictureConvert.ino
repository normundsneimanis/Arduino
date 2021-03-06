#include <avr/pgmspace.h>
#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library

/* Slider Controller ports */
#define cs_lcd   9
#define dc       10
#define rst      7
Adafruit_ST7735 tft = Adafruit_ST7735(cs_lcd, dc, rst);


//const unsigned short ARROW4_8c[209] PROGMEM={
//0x0000, 0x2124, 0x2124, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x2124, 0xFFFF, 0xFFFF, 0x0000, 0x0000,   // 0x0010 (16) pixels
//0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xDEDB, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0020 (32) pixels
//0x0000, 0x6B6D, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x6B6D, 0xFFFF, 0xFFFF,   // 0x0030 (48) pixels
//0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x6B6D, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000,   // 0x0040 (64) pixels
//0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x6B6D, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0050 (80) pixels
//0x0000, 0x6B6D, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x2124, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x6B6D, 0xFFFF, 0xFFFF,   // 0x0060 (96) pixels
//0xFFFF, 0xFFFF, 0x2124, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x9492, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000,   // 0x0070 (112) pixels
//0x0000, 0x0000, 0x0000, 0x4A49, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4A49, 0xFFFF, 0xFFFF,   // 0x0080 (128) pixels
//0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4A49, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x2124, 0x0000, 0x0000, 0x0000,   // 0x0090 (144) pixels
//0x0000, 0x4228, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4A49, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,   // 0x00A0 (160) pixels
//0x2124, 0x0000, 0x0000, 0x0000, 0x0000, 0x4A49, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x00B0 (176) pixels
//0xDEDB, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x2124, 0xFFFF, 0xFFFF, 0x0000, 0x0000,   // 0x00C0 (192) pixels
//0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x00D0 (208) pixels
//};
//
//const unsigned short REPEAT_8c[323] PROGMEM={
//0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x9492, 0x2124, 0x0000,   // 0x0010 (16) pixels
//0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0020 (32) pixels
//0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4A49, 0x4A49, 0x4A49, 0x4A49, 0x4A49, 0x4A49,   // 0x0030 (48) pixels
//0x4A49, 0x4A49, 0x4A49, 0xDEDB, 0xFFFF, 0xFFFF, 0x2124, 0x0000, 0x0000, 0x0000, 0x0000, 0x9492, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,   // 0x0040 (64) pixels
//0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x4A49, 0x0000, 0x0000, 0xB5B6, 0xFFFF, 0xFFFF,   // 0x0050 (80) pixels
//0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x6B6D, 0x0000, 0x4A49,   // 0x0060 (96) pixels
//0xFFFF, 0xFFFF, 0xFFFF, 0xB5B6, 0x9492, 0x9492, 0x9492, 0x9492, 0x9492, 0x9492, 0x9492, 0x9492, 0xFFFF, 0xFFFF, 0xFFFF, 0x6B6D,   // 0x0070 (112) pixels
//0x0000, 0x2124, 0xB5B6, 0xFFFF, 0xFFFF, 0x6B6D, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xDEDB,   // 0x0080 (128) pixels
//0xFFFF, 0x4A49, 0x0000, 0x4A49, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0090 (144) pixels
//0x0000, 0x0000, 0xDEDB, 0x6B6D, 0x0000, 0x2124, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x2124, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x00A0 (160) pixels
//0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xB5B6, 0x0000, 0x0000,   // 0x00B0 (176) pixels
//0x6B6D, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xDEDB,   // 0x00C0 (192) pixels
//0x0000, 0x0000, 0xB5B6, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF,   // 0x00D0 (208) pixels
//0xFFFF, 0x9492, 0x0000, 0x0000, 0xB5B6, 0xFFFF, 0xDEDB, 0x2124, 0x2124, 0x2104, 0x2124, 0x2124, 0x2124, 0x2104, 0x2104, 0x2124,   // 0x00E0 (224) pixels
//0xDEDB, 0xFFFF, 0xFFFF, 0xB5B6, 0x0000, 0x0000, 0xDEDB, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,   // 0x00F0 (240) pixels
//0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x2124, 0x0000, 0x6B6D, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,   // 0x0100 (256) pixels
//0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x4A49, 0x0000, 0x0000, 0x0000, 0x6B6D, 0xFFFF, 0xFFFF, 0xFFFF,   // 0x0110 (272) pixels
//0xB5B6, 0xB5B6, 0xB5B6, 0xB5B6, 0xB5B6, 0xB5B6, 0xB5B6, 0xB5B6, 0xB5B6, 0x9492, 0x2124, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0120 (288) pixels
//0x6B6D, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0130 (304) pixels
//0x0000, 0x0000, 0x0000, 0x0000, 0x6B6D, 0xDEDB, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0140 (320) pixels
//};
//
//const unsigned short START_DELAY_TIMER_8c[289] PROGMEM={
//0x0000, 0x0000, 0x4A49, 0xDEDB, 0xFFFF, 0xDEDB, 0x6B6D, 0x0000, 0x0000, 0x0000, 0x2124, 0xB5B6, 0xFFFF, 0xDEDB, 0x9492, 0x0000,   // 0x0010 (16) pixels
//0x0000, 0x0000, 0x6B6D, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xDEDB, 0x2124, 0x0000, 0x0000, 0x9492, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,   // 0x0020 (32) pixels
//0xDEDB, 0x0000, 0x4A49, 0xFFFF, 0xFFFF, 0xFFFF, 0xB5B6, 0x4A49, 0x4A49, 0x9492, 0xB5B6, 0x9492, 0x6B6D, 0x4A49, 0x6B6D, 0xFFFF,   // 0x0030 (48) pixels
//0xFFFF, 0xFFFF, 0xB5B6, 0xB5B6, 0xFFFF, 0xFFFF, 0x9492, 0x4A49, 0xDEDB, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x9492,   // 0x0040 (64) pixels
//0x4A49, 0xFFFF, 0xFFFF, 0xFFFF, 0xB5B6, 0xFFFF, 0x9492, 0x4A49, 0xFFFF, 0xFFFF, 0xB5B6, 0x6B6D, 0x2124, 0x4A49, 0x9492, 0xFFFF,   // 0x0050 (80) pixels
//0xFFFF, 0xB5B6, 0x4A49, 0xFFFF, 0xFFFF, 0x6B6D, 0xFFFF, 0x4A49, 0xFFFF, 0xFFFF, 0x6B6D, 0x0000, 0x0000, 0x2124, 0x0000, 0x0000,   // 0x0060 (96) pixels
//0x2104, 0xDEDB, 0xFFFF, 0x9492, 0x9492, 0xFFFF, 0x0000, 0x4228, 0xB5B6, 0xFFFF, 0x6B6D, 0x0000, 0x0000, 0x0000, 0xFFFF, 0x2124,   // 0x0070 (112) pixels
//0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x4A49, 0x2124, 0x0000, 0x2124, 0xFFFF, 0xDEDB, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF,   // 0x0080 (128) pixels
//0x2124, 0x0000, 0x0000, 0x0000, 0x6B6D, 0xFFFF, 0x9492, 0x0000, 0x0000, 0x6B6D, 0xFFFF, 0x9492, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0090 (144) pixels
//0xFFFF, 0x4A49, 0x0000, 0x0000, 0x0000, 0x2124, 0xFFFF, 0xDEDB, 0x0000, 0x0000, 0x6B6D, 0xFFFF, 0x9492, 0x0000, 0x0000, 0x0000,   // 0x00A0 (160) pixels
//0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x6B6D, 0x0000, 0xFFFF, 0xDEDB, 0x0000, 0x0000, 0x6B6D, 0xFFFF, 0x9492, 0x0000, 0x0000,   // 0x00B0 (176) pixels
//0x0000, 0x0000, 0x2124, 0x4A49, 0x4A49, 0x4228, 0x0000, 0x2124, 0xFFFF, 0xDEDB, 0x0000, 0x0000, 0x2124, 0xFFFF, 0xDEDB, 0x0000,   // 0x00C0 (192) pixels
//0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x6B6D, 0xFFFF, 0x9492, 0x0000, 0x0000, 0x0000, 0xDEDB, 0xFFFF,   // 0x00D0 (208) pixels
//0x6B6D, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x2124, 0xFFFF, 0xFFFF, 0x2124, 0x0000, 0x0000, 0x0000, 0x2124,   // 0x00E0 (224) pixels
//0xFFFF, 0xFFFF, 0x6B6D, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x2124, 0xDEDB, 0xFFFF, 0x9492, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x00F0 (240) pixels
//0x0000, 0x6B6D, 0xFFFF, 0xFFFF, 0xDEDB, 0x6B6D, 0x4A49, 0x4A49, 0x9492, 0xFFFF, 0xFFFF, 0xB5B6, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0100 (256) pixels
//0x0000, 0x0000, 0x0000, 0x4A49, 0xDEDB, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x9492, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0110 (272) pixels
//0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4A49, 0x9492, 0xB5B6, 0x9492, 0x6B6D, 0x2124, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0120 (288) pixels
//};

//const unsigned short ARROW5_8c[209] PROGMEM={
//0x0000, 0xF800, 0x2000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xF800, 0xF800, 0xF800, 0x4800, 0x0000,   // 0x0010 (16) pixels
//0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0020 (32) pixels
//0x0000, 0x2000, 0xF800, 0xF800, 0xF800, 0xF800, 0x6800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x6800, 0xF800, 0xF800,   // 0x0030 (48) pixels
//0xF800, 0xF800, 0x4800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4800, 0xF800, 0xF800, 0xF800, 0xF800, 0x2000, 0x0000,   // 0x0040 (64) pixels
//0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4800, 0xF800, 0xF800, 0xF800, 0xF800, 0x2000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0050 (80) pixels
//0x0000, 0x2000, 0xF800, 0xF800, 0xF800, 0xF800, 0x4800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x6800, 0xF800, 0xF800,   // 0x0060 (96) pixels
//0xF800, 0xF800, 0x4000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4800, 0xF800, 0xF800, 0xF800, 0xD800, 0x0000, 0x0000,   // 0x0070 (112) pixels
//0x0000, 0x0000, 0x0000, 0x2000, 0xF800, 0xF800, 0xF800, 0xF800, 0x6800, 0x0000, 0x0000, 0x0000, 0x0000, 0x2000, 0xF800, 0xF800,   // 0x0080 (128) pixels
//0xF800, 0xF800, 0x6800, 0x0000, 0x0000, 0x0000, 0x0000, 0x2000, 0xF800, 0xF800, 0xF800, 0xF800, 0x4000, 0x2000, 0x0000, 0x0000,   // 0x0090 (144) pixels
//0x0000, 0x6800, 0xF800, 0xF800, 0xF800, 0xF800, 0x4800, 0x0000, 0x0000, 0x0000, 0x0000, 0x2000, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x00A0 (160) pixels
//0x9000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0x4800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x00B0 (176) pixels
//0xF800, 0xF800, 0xF800, 0xF800, 0x6800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xF800, 0xF800, 0xF800, 0x4800, 0x0000,   // 0x00C0 (192) pixels
//0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xF800, 0x4000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x00D0 (208) pixels
//};

const uint8_t menuArrowYellowLeft[] PROGMEM = {
  0xB, 0x10, 0x20, 0xC, 0x10, 0x22, 0xA, 0x10, 0x24, 0x8, 0x10, 0x24, 0x10, 0x7, 0x10, 0x24, 
  0x10, 0x7, 0x10, 0x24, 0x10, 0x7, 0x10, 0x24, 0x10, 0x7, 0x10, 0x24, 0x10, 0x7, 0x10, 0x24, 
  0x30, 0x7, 0x10, 0x24, 0x10, 0x7, 0x10, 0x24, 0x10, 0x7, 0x10, 0x24, 0x10, 0x7, 0x10, 0x24, 
  0x10, 0x9, 0x10, 0x24, 0x10, 0x9, 0x10, 0x24, 0x10, 0x9, 0x10, 0x24, 0x10, 0x9, 0x30, 0x24, 
  0x10, 0x9, 0x30, 0x24, 0x10, 0x9, 0x30, 0x24, 0x10, 0x9, 0x10, 0x24, 0x10, 0x9, 0x10, 0x24, 
  0x10, 0x9, 0x10, 0x24, 0x10, 0x9, 0x10, 0x24, 0xA, 0x10, 0x22, 0xC, 0x10, 0x20,
};

const uint8_t menuArrowYellowRight[] PROGMEM = {
  0x1, 0x20, 0x10, 0xC, 0x22, 0x10, 0xA, 0x24, 0x10, 0x9, 0x10, 0x24, 0x10, 0x9, 0x10, 0x24, 
  0x10, 0x9, 0x10, 0x24, 0x10, 0x9, 0x10, 0x24, 0x10, 0x9, 0x10, 0x24, 0x10, 0x9, 0x30, 0x24, 
  0x10, 0x9, 0x10, 0x24, 0x10, 0x9, 0x10, 0x24, 0x10, 0x9, 0x10, 0x24, 0x10, 0x9, 0x10, 0x24, 
  0x10, 0x7, 0x10, 0x24, 0x10, 0x7, 0x10, 0x24, 0x10, 0x7, 0x10, 0x24, 0x10, 0x7, 0x10, 0x24, 
  0x30, 0x7, 0x10, 0x24, 0x30, 0x7, 0x10, 0x24, 0x30, 0x7, 0x10, 0x24, 0x10, 0x7, 0x10, 0x24, 
  0x10, 0x7, 0x10, 0x24, 0x10, 0x8, 0x24, 0x10, 0xA, 0x22, 0x10, 0xC, 0x20, 0x10,
};

const uint8_t menuArrowGreenLeft[] PROGMEM = {
  0xB, 0x40, 0x50, 0xC, 0x40, 0x52, 0xA, 0x40, 0x54, 0x8, 0x40, 0x54, 0x40, 0x7, 0x40, 0x54, 
  0x40, 0x7, 0x40, 0x54, 0x40, 0x7, 0x40, 0x54, 0x40, 0x7, 0x40, 0x54, 0x40, 0x7, 0x40, 0x54, 
  0x40, 0x7, 0x40, 0x54, 0x40, 0x7, 0x60, 0x54, 0x40, 0x7, 0x40, 0x54, 0x40, 0x7, 0x40, 0x54, 
  0x40, 0x9, 0x40, 0x54, 0x40, 0x9, 0x40, 0x54, 0x40, 0x9, 0x40, 0x54, 0x40, 0x9, 0x40, 0x54, 
  0x40, 0x9, 0x40, 0x54, 0x40, 0x9, 0x40, 0x54, 0x40, 0x9, 0x40, 0x54, 0x40, 0x9, 0x40, 0x54, 
  0x40, 0x9, 0x40, 0x54, 0x40, 0x9, 0x40, 0x54, 0xA, 0x40, 0x52, 0xC, 0x40, 0x50,
};

const uint8_t menuArrowGreenRight[] PROGMEM = {
  0x1, 0x50, 0x40, 0xC, 0x52, 0x40, 0xA, 0x54, 0x40, 0x9, 0x40, 0x54, 0x40, 0x9, 0x40, 0x54, 
  0x40, 0x9, 0x40, 0x54, 0x40, 0x9, 0x40, 0x54, 0x40, 0x9, 0x40, 0x54, 0x40, 0x9, 0x40, 0x54, 
  0x40, 0x9, 0x40, 0x54, 0x40, 0x9, 0x40, 0x54, 0x60, 0x9, 0x40, 0x54, 0x40, 0x9, 0x40, 0x54, 
  0x40, 0x7, 0x40, 0x54, 0x40, 0x7, 0x40, 0x54, 0x40, 0x7, 0x40, 0x54, 0x40, 0x7, 0x40, 0x54, 
  0x40, 0x7, 0x40, 0x54, 0x40, 0x7, 0x40, 0x54, 0x40, 0x7, 0x40, 0x54, 0x40, 0x7, 0x40, 0x54, 
  0x40, 0x7, 0x40, 0x54, 0x40, 0x8, 0x54, 0x40, 0xA, 0x52, 0x40, 0xC, 0x50, 0x40,
};

const uint8_t menuArrowRedLeft[] PROGMEM = {
  0xB, 0x70, 0x80, 0xC, 0x70, 0x82, 0xA, 0x70, 0x84, 0x8, 0x70, 0x84, 0x70, 0x7, 0x70, 0x84, 
  0x70, 0x7, 0x70, 0x84, 0x70, 0x7, 0x90, 0x84, 0x70, 0x7, 0x70, 0x84, 0x70, 0x7, 0x70, 0x84, 
  0x70, 0x7, 0x70, 0x84, 0x70, 0x7, 0x70, 0x84, 0x70, 0x7, 0x70, 0x84, 0x70, 0x7, 0x70, 0x84, 
  0x70, 0x9, 0x70, 0x84, 0x70, 0x9, 0x70, 0x84, 0x70, 0x9, 0x70, 0x84, 0x70, 0x9, 0x70, 0x84, 
  0x70, 0x9, 0x70, 0x84, 0x70, 0x9, 0x70, 0x84, 0x70, 0x9, 0x70, 0x84, 0x70, 0x9, 0x70, 0x84, 
  0x90, 0x9, 0x70, 0x84, 0x70, 0x9, 0x70, 0x84, 0xA, 0x70, 0x82, 0xC, 0x70, 0x80,
};

const uint8_t menuArrowRedRight[] PROGMEM = {
  0x1, 0x80, 0x70, 0xC, 0x82, 0x70, 0xA, 0x84, 0x70, 0x9, 0x70, 0x84, 0x70, 0x9, 0x70, 0x84, 
  0x70, 0x9, 0x70, 0x84, 0x70, 0x9, 0x70, 0x84, 0x90, 0x9, 0x70, 0x84, 0x70, 0x9, 0x70, 0x84, 
  0x70, 0x9, 0x70, 0x84, 0x70, 0x9, 0x70, 0x84, 0x70, 0x9, 0x70, 0x84, 0x70, 0x9, 0x70, 0x84, 
  0x70, 0x7, 0x70, 0x84, 0x70, 0x7, 0x70, 0x84, 0x70, 0x7, 0x70, 0x84, 0x70, 0x7, 0x70, 0x84, 
  0x70, 0x7, 0x70, 0x84, 0x70, 0x7, 0x70, 0x84, 0x70, 0x7, 0x70, 0x84, 0x70, 0x7, 0x90, 0x84, 
  0x70, 0x7, 0x70, 0x84, 0x70, 0x8, 0x84, 0x70, 0xA, 0x82, 0x70, 0xC, 0x80, 0x70,
};

const uint16_t arrowColorPalette[10] PROGMEM = {0x0, 0x6B60, 0xFFE0, 0x6B40, 0x360, 0x7E0, 0x340, 0x6800, 0xF800, 0x7000}; 

const uint8_t batteryFull[] PROGMEM = {
  0x0, 0x10, 0x2F, 0x2E, 0x10, 0x0, 0x31, 0x10, 0x2F, 0x2F, 0x20, 0x10, 0x31, 0x10, 0x20, 0x4F, 
  0x4E, 0x22, 0x0, 0x10, 0x20, 0x4F, 0x4E, 0x22, 0x11, 0x20, 0x4F, 0x4E, 0x22, 0x11, 0x20, 0x4F, 
  0x4E, 0x22, 0x11, 0x20, 0x4F, 0x4E, 0x22, 0x11, 0x20, 0x4F, 0x4E, 0x22, 0x0, 0x10, 0x2F, 0x2F, 
  0x20, 0x10, 0x31, 0x0, 0x10, 0x2F, 0x2E, 0x10, 0x0,
};

const uint8_t batteryMost[] PROGMEM = {
  0x0, 0x10, 0x2F, 0x2E, 0x10, 0x0, 0x31, 0x10, 0x2F, 0x2F, 0x20, 0x10, 0x31, 0x10, 0x20, 0x4F, 
  0x45, 0x38, 0x22, 0x0, 0x10, 0x20, 0x4F, 0x45, 0x38, 0x22, 0x11, 0x20, 0x4F, 0x45, 0x38, 0x22, 
  0x11, 0x20, 0x4F, 0x45, 0x38, 0x22, 0x11, 0x20, 0x4F, 0x45, 0x38, 0x22, 0x11, 0x20, 0x4F, 0x45, 
  0x38, 0x22, 0x0, 0x10, 0x2F, 0x2F, 0x20, 0x10, 0x31, 0x0, 0x10, 0x2F, 0x2E, 0x10, 0x0,
};

const uint8_t batteryThird[] PROGMEM = {
  0x0, 0x50, 0x2F, 0x2E, 0x10, 0x0, 0x31, 0x50, 0x2F, 0x2F, 0x20, 0x10, 0x31, 0x10, 0x20, 0x6A, 
  0x3F, 0x33, 0x22, 0x0, 0x10, 0x20, 0x6A, 0x3F, 0x33, 0x22, 0x11, 0x20, 0x6A, 0x3F, 0x33, 0x22, 
  0x11, 0x20, 0x6A, 0x3F, 0x33, 0x22, 0x11, 0x20, 0x6A, 0x3F, 0x33, 0x22, 0x11, 0x20, 0x6A, 0x3F, 
  0x33, 0x22, 0x0, 0x10, 0x2F, 0x2F, 0x20, 0x10, 0x31, 0x0, 0x10, 0x2F, 0x2E, 0x10, 0x0,
};

const uint8_t batteryEmpty[] PROGMEM = {
  0x0, 0x10, 0x2F, 0x2E, 0x10, 0x0, 0x31, 0x10, 0x2F, 0x2F, 0x20, 0x10, 0x31, 0x10, 0x20, 0x74, 
  0x3F, 0x39, 0x22, 0x0, 0x10, 0x20, 0x74, 0x3F, 0x39, 0x22, 0x11, 0x20, 0x74, 0x3F, 0x39, 0x22, 
  0x10, 0x80, 0x20, 0x74, 0x3F, 0x39, 0x22, 0x11, 0x20, 0x74, 0x3F, 0x39, 0x22, 0x11, 0x20, 0x74, 
  0x3F, 0x39, 0x22, 0x0, 0x10, 0x2F, 0x2F, 0x20, 0x10, 0x31, 0x0, 0x10, 0x2F, 0x2E, 0x10, 0x0, 
};

const uint16_t batteryColorPalette[9] PROGMEM = {0x6B6D, 0xDEDB, 0xFFFF, 0x0, 0x7E0, 0xD6DB, 0xFDA0, 0xF800, 0xDEDA};

const uint8_t modeSlider[] PROGMEM = {
  0xF, 0xA, 0x10, 0x20, 0x30, 0x43, 0x30, 0x20, 0xF, 0xF, 0xD, 0x20, 0x49, 0x50, 0xF, 0xD, 
  0x10, 0x60, 0x10, 0x9, 0x70, 0x44, 0x31, 0x44, 0x70, 0xF, 0x9, 0x80, 0x70, 0x44, 0x70, 0x60, 
  0x5, 0x30, 0x42, 0x30, 0x60, 0x2, 0x10, 0x50, 0x30, 0x42, 0x20, 0xF, 0x7, 0x20, 0x48, 0x70, 
  0x3, 0x50, 0x42, 0x20, 0x7, 0x20, 0x42, 0x60, 0xF, 0x5, 0x70, 0x43, 0x32, 0x43, 0x30, 0x2, 
  0x42, 0x20, 0x9, 0x20, 0x42, 0xF, 0x4, 0x20, 0x42, 0x20, 0x10, 0x2, 0x90, 0x20, 0x42, 0x20, 
  0x0, 0x50, 0x41, 0x30, 0xB, 0x42, 0x50, 0xF, 0x2, 0x10, 0x42, 0x50, 0x6, 0x60, 0x42, 0x0, 
  0x20, 0x41, 0x60, 0xB, 0x50, 0x41, 0x70, 0xF, 0x2, 0x70, 0x41, 0x30, 0x8, 0x70, 0x41, 0x70, 
  0x30, 0x41, 0xC, 0x10, 0x42, 0xF, 0x2, 0x42, 0x60, 0x9, 0x44, 0x30, 0xC, 0x90, 0x42, 0xF, 
  0x2, 0x42, 0xA, 0x30, 0x44, 0xD, 0x42, 0xF, 0x2, 0x42, 0xA, 0x30, 0x44, 0xC, 0x10, 0x42, 
  0xF, 0x2, 0x42, 0x10, 0x9, 0x41, 0x31, 0x41, 0x50, 0xB, 0x20, 0x41, 0x70, 0xF, 0x2, 0x30, 
  0x41, 0x50, 0x8, 0x80, 0x41, 0x70, 0x90, 0x42, 0xB, 0x42, 0x60, 0xF, 0x2, 0x50, 0x42, 0x8, 
  0x42, 0x50, 0x0, 0x30, 0x41, 0x70, 0x9, 0x30, 0x41, 0x30, 0xF, 0x4, 0x42, 0x30, 0x6, 0x30, 
  0x41, 0x30, 0x1, 0x10, 0x42, 0x30, 0x6, 0x10, 0x30, 0x42, 0x10, 0xF, 0x4, 0x10, 0x43, 0x20, 
  0x60, 0x11, 0x20, 0x43, 0x50, 0x0, 0x10, 0x0, 0x70, 0x43, 0x20, 0x60, 0x11, 0x60, 0x20, 0x43, 
  0x50, 0xF, 0x6, 0x50, 0x4F, 0x4D, 0x80, 0x6, 0x50, 0x41, 0x60, 0xB, 0x10, 0x30, 0x4F, 0x4C, 
  0x20, 0x5, 0x10, 0x70, 0x43, 0xA, 0x90, 0x43, 0x3F, 0x37, 0x43, 0x4, 0x50, 0x45, 0xA, 0x70, 
  0x41, 0x30, 0x10, 0xF, 0x7, 0x10, 0x30, 0x41, 0x70, 0x1, 0x10, 0x30, 0x42, 0x50, 0x20, 0x41, 
  0xA, 0x42, 0x10, 0xF, 0xA, 0x42, 0x0, 0x50, 0x42, 0x30, 0x80, 0x0, 0x20, 0x41, 0xA, 0x42, 
  0xF, 0xB, 0x30, 0x41, 0x30, 0x42, 0x20, 0x2, 0x20, 0x41, 0xA, 0x42, 0xF, 0xB, 0x30, 0x44, 
  0x60, 0x3, 0x20, 0x41, 0xA, 0x42, 0xF, 0xB, 0x30, 0x42, 0x70, 0x5, 0x20, 0x41, 0xA, 0x42, 
  0xF, 0xB, 0x30, 0x41, 0x60, 0x6, 0x20, 0x41, 0xA, 0x42, 0xF, 0xB, 0x30, 0x41, 0x7, 0x20, 
  0x41, 0xA, 0x42, 0xF, 0xB, 0x30, 0x41, 0x10, 0x6, 0x20, 0x41, 0xA, 0x42, 0xF, 0xB, 0x30, 
  0x41, 0x7, 0x20, 0x41, 0xA, 0x42, 0xF, 0xB, 0x30, 0x41, 0x10, 0x6, 0x20, 0x41, 0xA, 0x42, 
  0xF, 0xB, 0x30, 0x42, 0x10, 0x5, 0x20, 0x41, 0xA, 0x42, 0xF, 0xB, 0x30, 0x43, 0x60, 0x4, 
  0x20, 0x41, 0xA, 0x42, 0xF, 0xB, 0x30, 0x44, 0x50, 0x3, 0x20, 0x41, 0xA, 0x42, 0xF, 0xB, 
  0x30, 0x41, 0x10, 0x70, 0x41, 0x70, 0x2, 0x20, 0x41, 0xA, 0x42, 0xF, 0xB, 0x42, 0x1, 0x70, 
  0x41, 0x30, 0x1, 0x20, 0x41, 0xA, 0x42, 0x20, 0xF, 0x9, 0x50, 0x42, 0x2, 0x20, 0x42, 0x10, 
  0x50, 0x41, 0xA, 0x50, 0x42, 0x70, 0x6F, 0x67, 0x20, 0x42, 0x50, 0x3, 0x50, 0x42, 0x30, 0x41, 
  0xB, 0x30, 0x4F, 0x4E, 0x5, 0x60, 0x44, 0xC, 0x70, 0x4F, 0x4B, 0x30, 0x7, 0x10, 0x30, 0x41, 
  0x30, 0xD, 0x60, 0x2F, 0x29, 0x50, 0xA, 0x21, 0xF, 0xF, 0xF,
};

const uint8_t modeStopMotion[] PROGMEM = {
  0xF, 0xF, 0x2, 0x90, 0x20, 0x30, 0x41, 0x30, 0x20, 0x10, 0xF, 0xF, 0xF, 0x20, 0x45, 0x20, 
  0xF, 0xF, 0xF, 0x46, 0x70, 0xF, 0xF, 0xF, 0x46, 0x30, 0xF, 0xF, 0xF, 0x20, 0x45, 0x60, 
  0x3, 0x61, 0xF, 0xF, 0xA, 0x30, 0x43, 0x50, 0x3, 0x60, 0x41, 0x70, 0x10, 0xF, 0xF, 0x9, 
  0x10, 0x90, 0x10, 0x60, 0x4, 0x44, 0x60, 0xC, 0x10, 0x21, 0x80, 0xF, 0x5, 0x11, 0x62, 0x11, 
  0x3, 0x44, 0x50, 0xB, 0x60, 0x43, 0x50, 0xF, 0x1, 0x10, 0x20, 0x30, 0x47, 0x30, 0x20, 0x10, 
  0x0, 0x50, 0x41, 0x30, 0xC, 0x45, 0xF, 0x10, 0x70, 0x4D, 0x20, 0x1, 0x70, 0x60, 0xB, 0x10, 
  0x45, 0x60, 0xD, 0x20, 0x43, 0x30, 0x20, 0x60, 0x20, 0x41, 0x80, 0x60, 0x20, 0x30, 0x43, 0x60, 
  0xE, 0x45, 0x10, 0xC, 0x70, 0x42, 0x70, 0x90, 0x2, 0x60, 0x41, 0x3, 0x80, 0x70, 0x42, 0x20, 
  0xD, 0x60, 0x43, 0x20, 0xC, 0x30, 0x41, 0x30, 0x60, 0x5, 0x20, 0x50, 0x5, 0x60, 0x42, 0x70, 
  0xB, 0x10, 0x20, 0x42, 0x80, 0xC, 0x70, 0x41, 0x70, 0xF, 0x1, 0x30, 0x41, 0x20, 0x9, 0x50, 
  0x44, 0x10, 0xB, 0x20, 0x41, 0x30, 0xF, 0x3, 0x30, 0x41, 0x50, 0x6, 0x10, 0x30, 0x46, 0xA, 
  0x10, 0x42, 0xF, 0x5, 0x42, 0x5, 0x50, 0x48, 0x90, 0x9, 0x70, 0x41, 0x90, 0xF, 0x5, 0x60, 
  0x41, 0x20, 0x3, 0x70, 0x43, 0x50, 0x30, 0x43, 0x9, 0x10, 0x41, 0x70, 0x0, 0x70, 0x30, 0x60, 
  0xF, 0x60, 0x30, 0x70, 0x0, 0x30, 0x41, 0x2, 0x60, 0x42, 0x70, 0x1, 0x44, 0x9, 0x20, 0x41, 
  0x1, 0x42, 0x70, 0x10, 0xB, 0x90, 0x30, 0x42, 0x0, 0x10, 0x41, 0x50, 0x1, 0x60, 0x41, 0x50, 
  0x2, 0x44, 0x90, 0x8, 0x41, 0x30, 0x1, 0x50, 0x43, 0x20, 0x8, 0x10, 0x20, 0x43, 0x50, 0x1, 
  0x41, 0x30, 0x1, 0x50, 0x41, 0x3, 0x43, 0x30, 0x50, 0x30, 0x10, 0x5, 0x10, 0x41, 0x20, 0x2, 
  0x10, 0x70, 0x43, 0x50, 0x5, 0x20, 0x43, 0x20, 0x3, 0x70, 0x41, 0x1, 0x20, 0x41, 0x10, 0x2, 
  0x43, 0x30, 0x90, 0x41, 0x60, 0x4, 0x60, 0x41, 0x10, 0x4, 0x60, 0x30, 0x42, 0x30, 0x60, 0x1, 
  0x50, 0x43, 0x70, 0x10, 0x4, 0x60, 0x41, 0x60, 0x0, 0x70, 0x41, 0x3, 0x43, 0x30, 0x60, 0x42, 
  0x20, 0x3, 0x50, 0x41, 0x50, 0x10, 0x5, 0x50, 0x43, 0x20, 0x70, 0x42, 0x30, 0x80, 0x5, 0x90, 
  0x50, 0x41, 0x50, 0x0, 0x30, 0x41, 0x3, 0x43, 0x30, 0x0, 0x20, 0x42, 0x20, 0x2, 0x20, 0x43, 
  0x50, 0x6, 0x20, 0x45, 0x50, 0x6, 0x20, 0x43, 0x50, 0x0, 0x70, 0x40, 0x30, 0x2, 0x10, 0x44, 
  0x1, 0x50, 0x42, 0x2, 0x20, 0x43, 0x20, 0x7, 0x10, 0x70, 0x41, 0x20, 0x8, 0x70, 0x43, 0x50, 
  0x1, 0x50, 0x10, 0x3, 0x44, 0x3, 0x20, 0x50, 0x2, 0x50, 0x41, 0x50, 0x10, 0xA, 0x60, 0x90, 
  0xA, 0x60, 0x20, 0x41, 0x50, 0x8, 0x43, 0x30, 0x7, 0x80, 0x41, 0x10, 0xF, 0x9, 0x60, 0x41, 
  0x60, 0x6, 0x40, 0x10, 0x0, 0x43, 0x30, 0x6, 0x10, 0x41, 0x20, 0xF, 0x9, 0x70, 0x41, 0x6, 
  0x70, 0x41, 0x80, 0x0, 0x30, 0x42, 0x30, 0x6, 0x41, 0x30, 0xF, 0x9, 0x41, 0x30, 0x5, 0x60, 
  0x42, 0x30, 0x1, 0x30, 0x42, 0x30, 0x5, 0x20, 0x41, 0x10, 0xF, 0x7, 0x60, 0x41, 0x50, 0x5, 
  0x43, 0x3, 0x30, 0x42, 0x20, 0x4, 0x10, 0x41, 0x70, 0xF, 0x7, 0x30, 0x41, 0x5, 0x70, 0x42, 
  0x50, 0x4, 0x43, 0x5, 0x70, 0x41, 0x60, 0xF, 0x5, 0x50, 0x41, 0x20, 0x4, 0x10, 0x42, 0x30, 
  0x5, 0x20, 0x42, 0x80, 0x5, 0x42, 0xF, 0x4, 0x10, 0x42, 0x5, 0x43, 0x7, 0x42, 0x70, 0x5, 
  0x50, 0x41, 0x30, 0xF, 0x3, 0x42, 0x60, 0x4, 0x20, 0x42, 0x50, 0x7, 0x70, 0x42, 0x6, 0x70, 
  0x41, 0x30, 0xF, 0x0, 0x10, 0x42, 0x20, 0x4, 0x10, 0x42, 0x30, 0x8, 0x60, 0x42, 0x50, 0x6, 
  0x70, 0x42, 0x60, 0x5, 0x20, 0x50, 0x5, 0x50, 0x42, 0x70, 0x5, 0x30, 0x42, 0xA, 0x42, 0x70, 
  0x7, 0x70, 0x42, 0x70, 0x60, 0x2, 0x60, 0x41, 0x3, 0x60, 0x30, 0x42, 0x20, 0x5, 0x50, 0x42, 
  0x60, 0xA, 0x20, 0x42, 0x8, 0x50, 0x43, 0x30, 0x20, 0x80, 0x20, 0x41, 0x51, 0x70, 0x44, 0x60, 
  0x6, 0x42, 0x70, 0xC, 0x42, 0x50, 0x8, 0x10, 0x20, 0x4D, 0x20, 0x8, 0x70, 0x41, 0xD, 0x70, 
  0x41, 0x60, 0xA, 0x90, 0x20, 0x48, 0x30, 0x20, 0x10, 0xF, 0xF, 0xD, 0x11, 0x61, 0x11,
};

const uint8_t modeTimeLapse[] PROGMEM = {
  0xF, 0xF, 0x0, 0x60, 0x70, 0x30, 0x43, 0x30, 0x70, 0x60, 0xF, 0xF, 0xB, 0x50, 0x4B, 0x50, 
  0xF, 0xF, 0x7, 0x10, 0x30, 0x41, 0x30, 0x50, 0x10, 0x3, 0x10, 0x50, 0x30, 0x41, 0x30, 0x10, 
  0xF, 0xF, 0x4, 0x60, 0x41, 0x30, 0x60, 0x9, 0x60, 0x30, 0x41, 0x60, 0xF, 0xF, 0x2, 0x10, 
  0x41, 0x70, 0xD, 0x70, 0x41, 0x10, 0xF, 0xF, 0x1, 0x41, 0x20, 0x6, 0x21, 0x6, 0x70, 0x41, 
  0xF, 0xF, 0x0, 0x30, 0x40, 0x30, 0x6, 0x10, 0x41, 0x7, 0x30, 0x40, 0x70, 0xF, 0xE, 0x10, 
  0x41, 0x7, 0x10, 0x41, 0x8, 0x41, 0x10, 0xF, 0xD, 0x30, 0x40, 0x20, 0x7, 0x10, 0x41, 0x8, 
  0x20, 0x40, 0x70, 0xF, 0xD, 0x41, 0x8, 0x10, 0x41, 0x9, 0x41, 0xF, 0xC, 0x50, 0x40, 0x70, 
  0x8, 0x10, 0x41, 0x9, 0x30, 0x40, 0x50, 0xF, 0xB, 0x20, 0x40, 0x50, 0x8, 0x10, 0x41, 0x9, 
  0x20, 0x40, 0x20, 0xF, 0xB, 0x20, 0x40, 0x80, 0x8, 0x10, 0x41, 0x9, 0x50, 0x40, 0x20, 0xF, 
  0x4, 0x10, 0x50, 0x64, 0x30, 0x40, 0x60, 0x8, 0x10, 0x41, 0x50, 0x8, 0x60, 0x40, 0x70, 0xF, 
  0x4, 0x49, 0x10, 0x8, 0x70, 0x41, 0x50, 0x7, 0x60, 0x40, 0x20, 0xA, 0x20, 0x30, 0x41, 0x30, 
  0x50, 0x2, 0x30, 0x40, 0x30, 0x20, 0x74, 0x42, 0x9, 0x30, 0x41, 0x50, 0x6, 0x50, 0x40, 0x20, 
  0x9, 0x10, 0x45, 0x1, 0x70, 0x41, 0x7, 0x41, 0x30, 0x9, 0x30, 0x41, 0x20, 0x5, 0x70, 0x40, 
  0x50, 0x9, 0x10, 0x40, 0x30, 0x1, 0x41, 0x0, 0x60, 0x41, 0x60, 0x7, 0x10, 0x41, 0x20, 0x9, 
  0x30, 0x41, 0x20, 0x4, 0x41, 0x9, 0x10, 0x30, 0x41, 0x31, 0x44, 0x50, 0x9, 0x60, 0x44, 0x30, 
  0x60, 0x5, 0x41, 0x50, 0x3, 0x50, 0x40, 0x30, 0x8, 0x10, 0x49, 0x70, 0xB, 0x20, 0x45, 0x60, 
  0x5, 0x60, 0x4, 0x41, 0x60, 0x8, 0x30, 0x40, 0x70, 0x90, 0xA, 0x10, 0x50, 0x21, 0x50, 0x10, 
  0x7, 0x70, 0x40, 0x70, 0xA, 0x70, 0x40, 0x30, 0x9, 0x30, 0x40, 0x90, 0x9, 0x10, 0x70, 0x45, 
  0x30, 0x10, 0x5, 0x10, 0x40, 0x70, 0x9, 0x20, 0x41, 0xA, 0x30, 0x40, 0x10, 0x8, 0x60, 0x42, 
  0x70, 0x21, 0x70, 0x42, 0x50, 0x4, 0x10, 0x40, 0x70, 0x8, 0x20, 0x41, 0x60, 0xA, 0x30, 0x40, 
  0x10, 0x7, 0x10, 0x41, 0x70, 0x10, 0x3, 0x90, 0x70, 0x41, 0x60, 0x3, 0x10, 0x40, 0x70, 0x6, 
  0x10, 0x70, 0x41, 0x50, 0xB, 0x30, 0x40, 0x10, 0x7, 0x41, 0x70, 0x7, 0x70, 0x41, 0x3, 0x10, 
  0x40, 0x70, 0x4, 0x60, 0x70, 0x42, 0x10, 0xC, 0x30, 0x40, 0x10, 0x6, 0x60, 0x41, 0x9, 0x30, 
  0x40, 0x20, 0x2, 0x10, 0x41, 0x33, 0x43, 0x20, 0xE, 0x30, 0x40, 0x10, 0x6, 0x70, 0x40, 0x20, 
  0x9, 0x50, 0x40, 0x30, 0x2, 0x10, 0x46, 0x30, 0x50, 0x10, 0xF, 0x30, 0x40, 0x10, 0x6, 0x30, 
  0x40, 0x80, 0x9, 0x10, 0x41, 0x2, 0x10, 0x40, 0x70, 0x11, 0x90, 0x10, 0xF, 0x3, 0x30, 0x40, 
  0x10, 0x6, 0x30, 0x40, 0xB, 0x41, 0x2, 0x10, 0x40, 0x70, 0xF, 0x7, 0x30, 0x40, 0x10, 0x6, 
  0x30, 0x40, 0x60, 0x9, 0x10, 0x41, 0x2, 0x10, 0x40, 0x70, 0xF, 0x7, 0x30, 0x40, 0x10, 0x6, 
  0x70, 0x40, 0x70, 0x9, 0x50, 0x40, 0x30, 0x2, 0x90, 0x40, 0x70, 0xF, 0x7, 0x30, 0x40, 0x10, 
  0x6, 0x60, 0x41, 0x9, 0x41, 0x50, 0x2, 0x10, 0x40, 0x70, 0xF, 0x7, 0x30, 0x40, 0x10, 0x7, 
  0x30, 0x40, 0x30, 0x7, 0x30, 0x41, 0x3, 0x10, 0x40, 0x70, 0xF, 0x7, 0x30, 0x40, 0x10, 0x8, 
  0x41, 0x30, 0x60, 0x3, 0x60, 0x30, 0x41, 0x10, 0x3, 0x10, 0x40, 0x70, 0xF, 0x7, 0x30, 0x40, 
  0x10, 0x8, 0x10, 0x43, 0x71, 0x30, 0x42, 0x60, 0x4, 0x10, 0x40, 0x70, 0xF, 0x7, 0x30, 0x40, 
  0x10, 0xA, 0x20, 0x45, 0x20, 0x6, 0x10, 0x40, 0x70, 0xF, 0x7, 0x30, 0x40, 0x80, 0xC, 0x90, 
  0x61, 0x10, 0x8, 0x10, 0x40, 0x70, 0xF, 0x7, 0x30, 0x40, 0x30, 0x60, 0x90, 0x13, 0x90, 0x10, 
  0x90, 0x12, 0x5, 0x16, 0x60, 0x30, 0x40, 0x70, 0xF, 0x7, 0x10, 0x4F, 0x4D, 0x90, 0xF, 0x8, 
  0x90, 0x20, 0x70, 0x36, 0x70, 0x3D, 0x70, 0x30, 0x71, 0x10, 0xF, 0xF, 0xF, 0xF,
};

const uint8_t modeTimeLapsePro[] PROGMEM = {
  0xF, 0x7, 0x50, 0x41, 0x60, 0x4, 0x50, 0x70, 0x44, 0x30, 0x20, 0x60, 0xF, 0xF, 0x3, 0x60, 
  0x41, 0x60, 0x3, 0x20, 0x4A, 0x30, 0x60, 0xF, 0xF, 0x0, 0x90, 0x41, 0x60, 0x2, 0x60, 0x42, 
  0x70, 0x50, 0x10, 0x3, 0x60, 0x20, 0x42, 0x70, 0xF, 0xF, 0x30, 0x40, 0x50, 0x2, 0x50, 0x41, 
  0x30, 0x10, 0x9, 0x50, 0x41, 0x30, 0xF, 0xD, 0xA0, 0x40, 0x30, 0x2, 0x50, 0x41, 0x20, 0xD, 
  0x30, 0x41, 0xF, 0xC, 0x41, 0x10, 0x1, 0x60, 0x41, 0x50, 0xF, 0x30, 0x40, 0x30, 0xF, 0xA, 
  0x60, 0x40, 0x70, 0x2, 0x41, 0x20, 0xF, 0x1, 0x41, 0x50, 0xF, 0x9, 0x30, 0x40, 0x10, 0x1, 
  0x50, 0x40, 0x30, 0x1, 0x20, 0x70, 0x41, 0x70, 0x50, 0x3, 0x20, 0x70, 0x41, 0x70, 0x50, 0x0, 
  0x60, 0x41, 0xF, 0x9, 0x41, 0x2, 0x30, 0x40, 0x10, 0x0, 0x10, 0x45, 0x70, 0x1, 0x10, 0x45, 
  0x70, 0x0, 0x30, 0x40, 0x50, 0xF, 0x7, 0x80, 0x40, 0x20, 0x1, 0x10, 0x40, 0x30, 0x1, 0x80, 
  0x40, 0x30, 0x1, 0x60, 0x41, 0x10, 0x0, 0x60, 0x40, 0x30, 0x1, 0x80, 0x41, 0x11, 0x40, 0x30, 
  0xF, 0x7, 0x20, 0x40, 0x50, 0x1, 0x50, 0x40, 0x50, 0x1, 0x20, 0x40, 0x70, 0x2, 0x70, 0x40, 
  0x50, 0x0, 0x20, 0x40, 0x70, 0x2, 0x70, 0x40, 0x50, 0x0, 0x41, 0x50, 0xF, 0x6, 0x70, 0x40, 
  0x60, 0x1, 0x70, 0x40, 0x60, 0x1, 0x70, 0x40, 0x50, 0x2, 0x30, 0x40, 0x60, 0x0, 0x70, 0x40, 
  0x50, 0x2, 0x41, 0x10, 0x0, 0x30, 0x40, 0x20, 0xF, 0x6, 0x70, 0x40, 0x2, 0x30, 0x40, 0x2, 
  0x41, 0x2, 0x50, 0x41, 0x1, 0x41, 0x80, 0x0, 0x10, 0x30, 0x40, 0x20, 0x1, 0x70, 0x40, 0x20, 
  0xF, 0x4, 0x90, 0x60, 0x30, 0x40, 0x51, 0x80, 0x30, 0x40, 0x10, 0x0, 0x10, 0x41, 0x71, 0x42, 
  0x60, 0x1, 0x45, 0x50, 0x2, 0x70, 0x40, 0x70, 0xF, 0x3, 0x60, 0x49, 0x0, 0x10, 0x44, 0x70, 
  0x10, 0x1, 0x60, 0x41, 0x70, 0x30, 0x41, 0x90, 0x2, 0x70, 0x40, 0x20, 0xA, 0x70, 0x30, 0x41, 
  0x30, 0x60, 0x1, 0x10, 0x41, 0x70, 0x20, 0x74, 0x41, 0x20, 0x50, 0x40, 0x70, 0x90, 0x10, 0x4, 
  0x50, 0x40, 0x70, 0x1, 0x20, 0x40, 0x70, 0x2, 0x30, 0x40, 0x20, 0x9, 0x50, 0x45, 0x1, 0x30, 
  0x40, 0x30, 0x6, 0x10, 0x43, 0x50, 0x6, 0x70, 0x40, 0x50, 0x1, 0x10, 0x40, 0x30, 0x2, 0x41, 
  0x50, 0x9, 0x50, 0x40, 0x20, 0x0, 0x10, 0x41, 0x0, 0x20, 0x41, 0x8, 0x50, 0x42, 0x60, 0x6, 
  0x30, 0x40, 0x60, 0x1, 0x10, 0x41, 0x1, 0x10, 0x41, 0x9, 0x60, 0x30, 0x41, 0x31, 0x44, 0x10, 
  0x9, 0x20, 0x44, 0x70, 0x10, 0x1, 0x10, 0x41, 0x10, 0x1, 0x90, 0x41, 0x1, 0x70, 0x40, 0x20, 
  0x8, 0x60, 0x49, 0x50, 0xB, 0x30, 0x45, 0x90, 0x0, 0x10, 0x30, 0x70, 0x3, 0x70, 0x30, 0x0, 
  0x10, 0x41, 0x9, 0x30, 0x40, 0x20, 0xB, 0x60, 0x50, 0x21, 0x50, 0x10, 0x6, 0x10, 0x30, 0x40, 
  0x70, 0xA, 0x41, 0x20, 0x9, 0x30, 0x40, 0x10, 0x9, 0x60, 0x30, 0x45, 0x70, 0x90, 0x5, 0x10, 
  0x40, 0x70, 0x9, 0x70, 0x40, 0x30, 0xA, 0x30, 0x40, 0x10, 0x8, 0x20, 0x42, 0x70, 0x21, 0x30, 
  0x42, 0x80, 0x4, 0x10, 0x40, 0x70, 0x8, 0x70, 0x41, 0xB, 0x30, 0x40, 0x10, 0x7, 0x50, 0x41, 
  0x20, 0x4, 0x10, 0x30, 0x41, 0x10, 0x3, 0x60, 0x40, 0x70, 0x6, 0x60, 0x30, 0x41, 0x10, 0xB, 
  0x30, 0x40, 0x10, 0x7, 0x41, 0x20, 0x7, 0x30, 0x40, 0x30, 0x3, 0x80, 0x40, 0x70, 0x3, 0x10, 
  0x50, 0x70, 0x41, 0x30, 0x3, 0x70, 0x8, 0x30, 0x40, 0x10, 0x6, 0x20, 0x40, 0x30, 0x9, 0x41, 
  0x60, 0x2, 0x60, 0x41, 0x32, 0x44, 0x50, 0x3, 0x30, 0x40, 0x8, 0x30, 0x40, 0x10, 0x6, 0x30, 
  0x40, 0x60, 0x9, 0x70, 0x40, 0x70, 0x2, 0x60, 0x46, 0x70, 0x50, 0x4, 0x30, 0x40, 0x30, 0x8, 
  0x30, 0x40, 0x10, 0x6, 0x41, 0x60, 0x9, 0x50, 0x40, 0x30, 0x2, 0x60, 0x40, 0x70, 0x90, 0x12, 
  0x5, 0x50, 0x41, 0x30, 0x9, 0x30, 0x40, 0x10, 0x6, 0x41, 0xB, 0x40, 0x30, 0x2, 0x60, 0x40, 
  0x20, 0x7, 0x60, 0x30, 0x41, 0x20, 0xA, 0x30, 0x40, 0x10, 0x6, 0x41, 0x60, 0x9, 0x80, 0x40, 
  0x30, 0x2, 0x80, 0x40, 0x70, 0x4, 0x80, 0x20, 0x30, 0x41, 0x30, 0x80, 0xB, 0x30, 0x40, 0x10, 
  0x6, 0x30, 0x40, 0x80, 0x9, 0x70, 0x40, 0x20, 0x2, 0x60, 0x49, 0x30, 0x80, 0xD, 0x30, 0x40, 
  0x10, 0x6, 0x50, 0x40, 0x30, 0x8, 0x10, 0x41, 0x10, 0x2, 0x80, 0x45, 0x30, 0x70, 0x50, 0x10, 
  0xF, 0x30, 0x40, 0x10, 0x7, 0x41, 0x70, 0x7, 0x41, 0x70, 0x3, 0x60, 0x40, 0x70, 0xF, 0x7, 
  0x30, 0x40, 0x10, 0x7, 0x80, 0x41, 0x70, 0x10, 0x3, 0x60, 0x42, 0x4, 0x60, 0x40, 0x70, 0xF, 
  0x7, 0x30, 0x40, 0x90, 0x8, 0x50, 0x42, 0xB0, 0x71, 0x42, 0x30, 0x10, 0x4, 0x60, 0x40, 0x70, 
  0xF, 0x7, 0x30, 0x40, 0x10, 0x9, 0x10, 0x70, 0x45, 0x20, 0x6, 0x10, 0x40, 0x70, 0xF, 0x7, 
  0x30, 0x40, 0x10, 0xC, 0x10, 0x61, 0x10, 0x8, 0x60, 0x40, 0x70, 0xF, 0x7, 0x30, 0x40, 0x70, 
  0x1B, 0x5, 0x16, 0x60, 0x30, 0x40, 0x70, 0xF, 0x7, 0x10, 0x4F, 0x4D, 0x10, 0xF, 0x8, 0x60, 
  0x72, 0x3F, 0x31, 0x70, 0x33, 0x70, 0x20, 0x10, 0xF, 0xF, 0xF, 0xF,
};

const uint16_t menuColorPalette[12] PROGMEM = {0x0, 0x2124, 0x9492, 0xDEDB, 0xFFFF, 0x6B6D, 0x4A49, 0xB5B6, 0x4228, 0x2104, 0x6B4D, 0xDEFB}; 


// 11x19
const uint8_t iconArrowRight[] PROGMEM = {
  0x0, 0x11, 0x7, 0x10, 0x21, 0x7, 0x22, 0x30, 0x6, 0x40, 0x23, 0x6, 0x40, 0x23, 0x6, 0x40, 
  0x23, 0x6, 0x40, 0x23, 0x6, 0x40, 0x23, 0x10, 0x5, 0x40, 0x23, 0x10, 0x5, 0x50, 0x23, 0x4, 
  0x60, 0x23, 0x4, 0x60, 0x23, 0x4, 0x60, 0x23, 0x10, 0x3, 0x70, 0x23, 0x4, 0x60, 0x23, 0x10, 
  0x3, 0x60, 0x23, 0x5, 0x30, 0x22, 0x6, 0x10, 0x21, 0xF,
};

// 19x17
const uint8_t iconRepeat[] PROGMEM = {
  0xC, 0x50, 0x10, 0xF, 0x0, 0x21, 0x7, 0x68, 0x30, 0x21, 0x10, 0x3, 0x50, 0x2D, 0x60, 0x1, 
  0x80, 0x2E, 0x40, 0x0, 0x60, 0x22, 0x80, 0x57, 0x22, 0x40, 0x0, 0x10, 0x80, 0x21, 0x40, 0x8, 
  0x30, 0x20, 0x60, 0x0, 0x60, 0x23, 0x9, 0x30, 0x40, 0x0, 0x10, 0x24, 0x10, 0xB, 0x24, 0x80, 
  0x1, 0x40, 0x9, 0x23, 0x30, 0x1, 0x80, 0x20, 0x9, 0x22, 0x50, 0x1, 0x80, 0x20, 0x30, 0x11, 
  0x90, 0x12, 0x91, 0x10, 0x30, 0x21, 0x80, 0x1, 0x30, 0x2E, 0x10, 0x0, 0x40, 0x2E, 0x60, 0x2, 
  0x40, 0x22, 0x88, 0x50, 0x10, 0x4, 0x40, 0x21, 0xF, 0x0, 0x40, 0x30,
};

const uint8_t iconAlarm[] PROGMEM = {
  0x1, 0x60, 0x30, 0x20, 0x30, 0x40, 0x2, 0x10, 0x80, 0x20, 0x30, 0x50, 0x2, 0x40, 0x23, 0x30, 
  0x10, 0x1, 0x50, 0x23, 0x30, 0x0, 0x60, 0x22, 0x80, 0x61, 0x50, 0x80, 0x50, 0x40, 0x60, 0x40, 
  0x22, 0x81, 0x21, 0x50, 0x60, 0x30, 0x25, 0x50, 0x60, 0x22, 0x80, 0x20, 0x50, 0x60, 0x21, 0x80, 
  0x40, 0x10, 0x60, 0x50, 0x21, 0x80, 0x60, 0x21, 0x40, 0x20, 0x60, 0x21, 0x40, 0x1, 0x10, 0x1, 
  0x90, 0x30, 0x20, 0x51, 0x20, 0x0, 0x70, 0x80, 0x20, 0x40, 0x2, 0x20, 0x10, 0x2, 0x21, 0x60, 
  0x10, 0x0, 0x10, 0x20, 0x30, 0x3, 0x20, 0x10, 0x2, 0x40, 0x20, 0x50, 0x1, 0x40, 0x20, 0x50, 
  0x3, 0x20, 0x60, 0x2, 0x10, 0x20, 0x30, 0x1, 0x40, 0x20, 0x50, 0x3, 0x23, 0x40, 0x0, 0x20, 
  0x30, 0x1, 0x40, 0x20, 0x50, 0x3, 0x10, 0x61, 0x70, 0x0, 0x10, 0x20, 0x30, 0x1, 0x10, 0x20, 
  0x30, 0x8, 0x40, 0x20, 0x50, 0x2, 0x30, 0x20, 0x40, 0x6, 0x10, 0x21, 0x10, 0x2, 0x10, 0x21, 
  0x40, 0x4, 0x10, 0x30, 0x20, 0x50, 0x4, 0x40, 0x21, 0x30, 0x40, 0x61, 0x50, 0x21, 0x80, 0x6, 
  0x60, 0x30, 0x25, 0x50, 0x9, 0x60, 0x50, 0x80, 0x50, 0x40, 0x10,
};

const uint16_t iconColorPalette[10] PROGMEM = { 0x0, 0x2124, 0xFFFF, 0xDEDB, 0x6B6D, 0x9492, 0x4A49, 0x4228, 0xB5B6, 0x2104, }; 

const uint8_t iconArrowSelected[] PROGMEM = {
  0x0, 0x10, 0x20, 0x7, 0x12, 0x30, 0x6, 0x13, 0x6, 0x20, 0x13, 0x40, 0x5, 0x40, 0x13, 0x30, 
  0x5, 0x30, 0x13, 0x20, 0x5, 0x30, 0x13, 0x20, 0x5, 0x20, 0x13, 0x30, 0x5, 0x40, 0x13, 0x50, 
  0x5, 0x30, 0x12, 0x60, 0x4, 0x20, 0x13, 0x40, 0x3, 0x20, 0x13, 0x40, 0x3, 0x20, 0x13, 0x50, 
  0x20, 0x2, 0x40, 0x13, 0x30, 0x3, 0x20, 0x13, 0x70, 0x4, 0x13, 0x30, 0x4, 0x13, 0x40, 0x5, 
  0x12, 0x30, 0x7, 0x10, 0x50,
};

const uint16_t iconArrowSelectedColorPalette[8] PROGMEM = { 0x0, 0xF800, 0x2000, 0x4800, 0x6800, 0x4000, 0xD800, 0x9000, };


colorZip_t currentColor;
//currentColor.color = (0x1B >> 4); // first 4 bytes (0xA)
//currentColor.color = (0xAF & 0xF); // last 4 bits (0xB)


#define IMAGE_SIZE 209

uint16_t colorPalette[32];
int colorIdx = 0;

uint8_t record;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1); // 0 - Portrait, 1 - Landscape
  tft.fillScreen(ST7735_BLACK);
  //tft.drawBitmap(2, 3, 37, 10, BATTERY_100_8c);
// Normal
//tft.drawBitmap(5, 5, cameramode, 45, 40, ST7735_WHITE);
  //tft.drawBitmap(50, 20, batteryFull, 37, 10, ST7735_WHITE);
  tft.drawBitmap(0, 0, 37, 10, batteryFull, batteryColorPalette);
  tft.drawBitmap(0, 10, 37, 10, batteryMost, batteryColorPalette);
  tft.drawBitmap(0, 20, 37, 10, batteryThird, batteryColorPalette);
  tft.drawBitmap(0, 30, 37, 10, batteryEmpty, batteryColorPalette);

  tft.drawBitmap(40, 0, 16, 25, menuArrowYellowLeft, arrowColorPalette);
  tft.drawBitmap(40, 25, 16, 25, menuArrowYellowRight, arrowColorPalette);
  tft.drawBitmap(65, 0, 16, 25, menuArrowGreenLeft, arrowColorPalette);
  tft.drawBitmap(65, 25, 16, 25, menuArrowGreenRight, arrowColorPalette);
  tft.drawBitmap(90, 0, 16, 25, menuArrowRedLeft, arrowColorPalette);
  tft.drawBitmap(90, 25, 16, 25, menuArrowRedRight, arrowColorPalette);

  tft.drawBitmap(106, 0, 11, 19, iconArrowRight, iconColorPalette); 
  tft.drawBitmap(134, 0, 11, 19, iconArrowSelected, iconArrowSelectedColorPalette); 
  tft.drawBitmap(117, 0, 19, 17, iconRepeat, iconColorPalette);
  tft.drawBitmap(117, 19, 17, 17, iconAlarm, iconColorPalette);
  tft.drawBitmap(117, 19, 17, 17, iconAlarm, iconColorPalette);

  tft.drawBitmap(0, 50, 56, 41, modeSlider, menuColorPalette); 
  tft.drawBitmap(56, 50, 56, 41, modeStopMotion, menuColorPalette); 
  tft.drawBitmap(112, 50, 56, 41, modeTimeLapse, menuColorPalette);
  tft.drawBitmap(0, 91, 56, 41, modeTimeLapsePro, menuColorPalette);


//
//  tft.drawBitmap(40, 25, menuArrowRight, 16, 25, ST7735_WHITE, menuArrowGreenRight, colorPalette);
//  tft.drawBitmap(65, 25, menuArrowRight, 16, 25, ST7735_WHITE, menuArrowRedRight, colorPalette);
//  tft.drawBitmap(90, 25, menuArrowRight, 16, 25, ST7735_WHITE, menuArrowYellowRight, colorPalette);
  
  
////  tft.drawBitmapOffsets(0,0, 56, 41, offsetTable, colorTable);
//  tft.drawBitmapOffsets(0,0, 56, 40, colorTable);

//  uint16_t totalRecords = 0;
//  uint16_t bo;
//  int foundColor;
//  int printIterator = 0;
//
//  currentColor.color = 0;
//  currentColor.count = 0;
//  bool startedColor = 0;
//  for (uint16_t i = 0; i < IMAGE_SIZE; i++) {
//    bo = pgm_read_word(&ARROW5_8c[i]);
////    Serial.print("Color ");
////    Serial.print(i);
////    Serial.print(" is ");
////    Serial.println((uint16_t) bo, HEX);
//  
//    // Color compression algorhytm
//    foundColor = -1;
//    for (int c = 0; c < colorIdx; c++) {
//      if (colorPalette[c] == (uint16_t) bo) {
////        Serial.print("Color found, index: ");
////        Serial.println(c);
//        foundColor = c;
//      }
//    }
//    if (foundColor == -1) {
////      Serial.println("Color not found, adding to palette");
//      foundColor = colorIdx;
//      colorPalette[colorIdx++] = (uint16_t) bo;
//    }
//
//    if (!startedColor) {
//      currentColor.color = foundColor;
//      currentColor.count = 0;
//      startedColor = 1;
//      totalRecords++;
//    } else {
//      if (colorPalette[currentColor.color] == (uint16_t) bo && currentColor.count < 15) {
//        currentColor.count++;
//        // Continue to the next pixel
//        continue;
//      }
//      if ((currentColor.count == 15) || (colorPalette[currentColor.color] != (uint16_t) bo) || (i == (IMAGE_SIZE - 1))) {
//        printIterator++;
//        record = 0;
//        record = ((currentColor.color << 4) | (currentColor.count & 0xF));
//        Serial.print("0x");
//        Serial.print(record, HEX);
////        Serial.print("{0x");
////        Serial.print(currentColor.color, HEX);
////        Serial.print(", 0x");
////        Serial.print(currentColor.count, HEX);
//        if (printIterator == 16) {
////          Serial.println("}, ");
//          Serial.println(", ");
//          printIterator = 0;
//        } else {
////          Serial.print("}, ");
//          Serial.print(", ");
//        }
//        totalRecords++;
//        currentColor.color = foundColor;
//        currentColor.count = 0;
//      }
//    }
//  }
//  Serial.print("Total records: ");
//  Serial.print(totalRecords);
//  Serial.println(" (bytes).");
//  Serial.print("Current color index: ");
//  Serial.println(colorIdx);
//  Serial.print("Color palette: ");
//  Serial.print("{ ");
//  for (int i = 0; i < colorIdx; i++) {
//    Serial.print("0x");
//    Serial.print(colorPalette[i], HEX);
//    Serial.print(", ");
//  }
//  Serial.print("}; ");
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
