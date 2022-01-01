/*
  SD card datalogger

 This example shows how to log data from three analog sensors
 to an SD card using the SD library.

 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4

 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */

#include <SPI.h>
#include <SD.h>

#define MAX_STRING 150

const int sdCardSelectPort = 10;
bool sdCardAvailable = false;

unsigned char requestStringIterator;
char requestString[MAX_STRING];
String dataString = "";
File dataFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print(F("Initializing SD card..."));

  // see if the card is present and can be initialized:
  if (!SD.begin(sdCardSelectPort)) {
    Serial.println(F("Card failed, or not present"));
  } else {
    Serial.println(F("card initialized."));
    sdCardAvailable = true;
  }
}

void loop() {
  // make a string for assembling the data to log:
  dataString = "";

  requestStringIterator = 0;
  memset(requestString, 0, MAX_STRING);
  while (Serial.available()) {
    requestString[requestStringIterator++] = Serial.read();
  }
  if (requestStringIterator) {
    requestString[requestStringIterator++] = '\r';
    requestString[requestStringIterator++] = '\n';
    Serial.print(F("Writing: ["));
    Serial.print(requestString);
    Serial.print(F("]\n"));
    dataFile = SD.open(F("datalog.txt"), FILE_WRITE);
    if (dataFile) {
      dataFile.print(requestString);
      dataFile.close();
    } else {
      Serial.println(F("error opening datalog.txt"));
    }
  }
  delay(200);
}
