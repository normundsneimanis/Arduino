#include <EEPROM.h>
#include <stdio.h>
#include <string.h>

// Contains request string
char requestString[16];
int requestStringIterator;

// Strings used for requested operation string comparison
const char *readString = "read";
const char *writeString = "write";

void setup() {
  Serial.begin(9600);
}

// Returns length of an integer when converted to a string
int intLength(int address) {
  char len[10];
  sprintf(len, "%d", address);
  return strlen(len);
}

void loop() {
  requestStringIterator = 0;
  while (Serial.available()) {
    requestString[requestStringIterator++] = Serial.read();
  }
  if (requestStringIterator) {
    requestString[requestStringIterator++] = '\0';
    Serial.print(requestString);
    Serial.print("\n");
    if (strncmp(readString, requestString, 4) == 0) {
      Serial.print("Received *read* operation request. Address: [");
      int address = atoi(&requestString[4]);
      Serial.print(address);
      Serial.print("] Content: [");
      Serial.print(EEPROM.read(address));
      Serial.print("]\n");
    } else if (strncmp(writeString, requestString, 5) == 0) {
      Serial.print("Received *write* operation request. Adress: [");
      int address = atoi(&requestString[5]);
      // Some pointer arithmentic is used to get to data that needs to be written
      int data = atoi(&requestString[5 + intLength(address)+1]);
      Serial.print(address);
      Serial.print("] Content: [");
      Serial.print(data);
      Serial.print("]\n");
      EEPROM.write(address, data);
    } else {
      Serial.print("Invalid operation request\n");
    }
  }
  delay(200);
}
