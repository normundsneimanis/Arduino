#include <string.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "solarPacket.h"
const byte CHIP_ENABLE = 9;
const byte CHIP_SELECT = 10;

#define STATUSLED 5
RF24 radio(CHIP_ENABLE, CHIP_SELECT);

const uint64_t pipes[2] = { 0xc897f8d075LL, 0x5feddd192fLL };
const uint64_t pipe = 0xc897f8d072LL;
#define MAX_TEXT 16

solarPacket packet;


void setup()
{
  Serial.begin(115200);
  pinMode(STATUSLED, OUTPUT);
  SPI.begin();
  digitalWrite (CHIP_ENABLE, LOW); 
  digitalWrite (CHIP_SELECT, HIGH);
  radio.begin();
  radio.openWritingPipe(pipe);
  digitalWrite(STATUSLED, HIGH);
  delay(20);
  digitalWrite(STATUSLED, LOW);
  Serial.println(F("Sender starting."));
  radio.printDetails();
}

int counter = 0;

bool returnVal;

void loop()
{
  packet.temperature = counter++;
  packet.humidity = counter++;
  packet.voltage = counter++;
  if (counter > 95) {
    counter = 0;
  }
  returnVal = radio.write(&packet, sizeof packet);
  Serial.println(returnVal);
  if (returnVal) {
    digitalWrite(STATUSLED, HIGH);
    delay(20);
    digitalWrite(STATUSLED, LOW);
  }
  
  delay(1000);
}
