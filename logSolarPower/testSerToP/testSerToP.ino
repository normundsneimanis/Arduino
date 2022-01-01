/*
  Shift Register Example
 for 74HC595 shift register

 This sketch turns reads serial input and uses it to set the pins
 of a 74HC595 shift register.

 Hardware:
 * 74HC595 shift register attached to pins 8, 12, and 11 of the Arduino,
 as detailed below.
 * LEDs attached to each of the outputs of the shift register.


 Created 22 May 2009
 Created 23 Mar 2010
 by Tom Igoe

 */

//Pin connected to latch pin (ST_CP) of 74HC595
const int latchPin = A1;
//Pin connected to clock pin (SH_CP) of 74HC595
const int clockPin = A2;
////Pin connected to Data in (DS) of 74HC595
const int dataPin = A3;

#define SERIALBUFFERSIZE 512
static char serialBuffer[SERIALBUFFERSIZE];

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  Serial.begin(115200);
  Serial.println("starting");
  int byteToSend = 0;
  byteToSend |= 1 << 15;
  Serial.print("Setting bits to high: ");
  Serial.println(portsEnable);
  registerWrite(byteToSend);
}

void loop() {
  if (Serial.available() > 0) {
    unsigned int portsEnable = 0;
    int index = 0;
    memset(serialBuffer,0,SERIALBUFFERSIZE);
    while (Serial.available()) {
      serialBuffer[index++] = Serial.read();
      delay(1);
    }
    portsEnable = atoi(serialBuffer);
    Serial.print("Setting bits to high: ");
    Serial.println(portsEnable);
    registerWrite(portsEnable);
  }
}

void registerWrite(unsigned int portsEnable) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, portsEnable >> 8);
    shiftOut(dataPin, clockPin, MSBFIRST, portsEnable);
    digitalWrite(latchPin, HIGH);
}
