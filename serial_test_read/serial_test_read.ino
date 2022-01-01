#include <SoftwareSerial.h>

/*
  Reading a serial ASCII-encoded string.

 This sketch demonstrates the Serial parseInt() function.
 It looks for an ASCII string of comma-separated values.
 It parses them into ints, and uses those to fade an RGB LED.

 Circuit: Common-anode RGB LED wired like so:
 * Red cathode: digital pin 3
 * Green cathode: digital pin 5
 * blue cathode: digital pin 6
 * anode: +5V

 created 13 Apr 2012
 by Tom Igoe

 This example code is in the public domain.
 */

#define SERIALBUFFERSIZE 512
#define rxPin 2
#define txPin 3
#define ledPin 13
static char serialBuffer[SERIALBUFFERSIZE];
int pinState = 0;

SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);


void setup() {
  // initialize serial:
  
  //Serial.begin(115200);
  Serial.begin(115200);
 pinMode(ledPin, OUTPUT);
 // set the data rate for the SoftwareSerial port
 mySerial.begin(115200);

}

void toggle(int pinNum) {
 // set the LED pin using the pinState variable:
 digitalWrite(pinNum, pinState);
 // if pinState = 0, set it to 1, and vice versa:
 pinState = !pinState;
}

void loop() {
  // if there's any serial available, read it:
  int index = 0;
  memset(serialBuffer,0,SERIALBUFFERSIZE);
  while (mySerial.available()) {
    serialBuffer[index++] = mySerial.read();
  }
  if (index) {
    Serial.print("Red bytes ");
    Serial.println(index + 1);
    toggle(13);
  }
}








