#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define STATUSLED 4

RF24 radio(7, 8);

const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
#define MAX_TEXT 22
char text[MAX_TEXT];

void setup()
{
  while (!Serial);
  Serial.begin(115200);
  pinMode(STATUSLED, OUTPUT);
  radio.begin();
  radio.openReadingPipe(0, pipes[0]);
  digitalWrite(STATUSLED, HIGH);
  delay(20);
  digitalWrite(STATUSLED, LOW);
  radio.startListening();
}

void loop()
{
  if (radio.available())
  {
    digitalWrite(STATUSLED, HIGH);
    delay(20);
    digitalWrite(STATUSLED, LOW);
    memset(text, 0, MAX_TEXT);
    radio.read(&text, MAX_TEXT);
    Serial.print("Received: ");
    Serial.println(text);
  }
  //Serial.println("Line");
  delay(10);
}
