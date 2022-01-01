#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "solarPacket.h"

#define STATUSLED 4

RF24 radio(9, 10);

const uint64_t pipes[2] = { 0xc897f8d075LL, 0x5feddd192fLL };
const uint64_t pipe = 0xc897f8d072LL;

void setup()
{
  Serial.begin(115200);
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  pinMode(STATUSLED, OUTPUT);
  radio.begin();
  radio.openReadingPipe(1, pipe);
  radio.startListening();
  digitalWrite(STATUSLED, HIGH);
  delay(20);
  digitalWrite(STATUSLED, LOW);
  Serial.println(F("Receiver starting."));
  radio.printDetails();
}

solarPacket packet;

void loop()
{
  if (radio.available())
  {
    digitalWrite(STATUSLED, HIGH);
    delay(20);
    digitalWrite(STATUSLED, LOW);
    radio.read( &packet, sizeof packet );
    Serial.print(F(" Temp: "));
    Serial.print(packet.temperature);
    Serial.print(F(" Voltage: "));
    Serial.print(packet.voltage);
    Serial.print(F(", Humidity: "));
    Serial.println(packet.humidity);
  }
  //Serial.println("Line");
  delay(10);
}
