#include <string.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
const byte CHIP_ENABLE = 9;
const byte CHIP_SELECT = 10;

#define STATUSLED 3
RF24 radio(CHIP_ENABLE, CHIP_SELECT);

const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
#define MAX_TEXT 16

char text[MAX_TEXT];

void setup()
{
  pinMode(STATUSLED, OUTPUT);
  SPI.begin ();
  digitalWrite (CHIP_ENABLE, LOW); 
  digitalWrite (CHIP_SELECT, HIGH);
  radio.begin();
  radio.setRetries(15, 15);
  radio.openWritingPipe(pipes[0]);
  radio.setPayloadSize(8);
  radio.startListening ();
  delay (10);
  radio.stopListening ();
  memset(text, 0, MAX_TEXT);
  strcpy(text, " Hello World    ");
  Serial.begin(115200);
  digitalWrite(STATUSLED, HIGH);
  delay(20);
  digitalWrite(STATUSLED, LOW);
  radio.stopListening();
}

int counter = 0;

bool returnVal;

void loop()
{
  text[13] = 48 + counter++;
  if (counter > 75) {
    counter = 0;
  }
  returnVal = radio.write(&text, MAX_TEXT);
  Serial.print(returnVal);
  Serial.println(text);
  if (returnVal) {
    digitalWrite(STATUSLED, HIGH);
    delay(20);
    digitalWrite(STATUSLED, LOW);
  }
  
  delay(1000);
}
