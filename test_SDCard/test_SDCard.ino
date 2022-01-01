#include <stdio.h>
#include <string.h>
#include <SPI.h>
#include <SD.h>

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

const int chipSelect = 2;

// Contains request string
//char requestString[255];
int requestStringIterator;

// Strings used for requested operation string comparison
const char *readString = "read";
const char *writeString = "write";

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  Serial.print("\nInitializing SD card...");

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }


  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("\nVolume type is FAT");
  Serial.println(volume.fatType(), DEC);
  Serial.println();

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
  Serial.print("Volume size (bytes): ");
  Serial.println(volumesize);
  Serial.print("Volume size (Kbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Mbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);


  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
}

// Returns length of an integer when converted to a string
int intLength(int address) {
  char len[10];
  sprintf(len, "%d", address);
  return strlen(len);
}

void loop() {
//  requestStringIterator = 0;
//  while (Serial.available()) {
//    requestString[requestStringIterator++] = Serial.read();
//  }
//  if (requestStringIterator) {
//    requestString[requestStringIterator++] = '\0';
//    Serial.print("Writing: [");
//    Serial.print(requestString);
//    Serial.print("]\n");
//  }
//  delay(200);
}
