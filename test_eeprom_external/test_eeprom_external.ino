#include <Wire.h>    

/*
 * Using the following example
 * http://www.hobbytronics.co.uk/arduino-external-eeprom
 * 
 * This example writes/reads one byte at the time
 * 
 * Using 24LC256 chip
 * 
 * Connections:
 * GND - pins 3,4,7
 * +5V (2.5-5.5V) 8
 * A5 - SCL (6)
 * A4 - SDA (5)
 * 
 */

#define disk1 0x50    //Address of 24LC256 eeprom chip
 
void setup(void)
{
  Serial.begin(115200);
  Wire.begin();  
 
  unsigned int address = 0;
 
  writeEEPROM(disk1, address, 123);
  Serial.print(readEEPROM(disk1, address), DEC);
}
 
void loop(){}
 
void writeEEPROM(int deviceaddress, unsigned int eeaddress, byte data ) 
{
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));   // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.write(data);
  Wire.endTransmission();
 
  delay(5);
}
 
byte readEEPROM(int deviceaddress, unsigned int eeaddress ) 
{
  byte rdata = 0xFF;
 
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));   // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
 
  Wire.requestFrom(deviceaddress,1);

  if (Wire.available()) rdata = Wire.read();
 
  return rdata;
}
