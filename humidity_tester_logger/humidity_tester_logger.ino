#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN            5         // Pin which is connected to the DHT sensor.
#define DHTTYPE           DHT11     // DHT 11

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <stdio.h>
#include <string.h>
#include <SPI.h>
#include <SD.h>

#define chipSelect 2

Sd2Card card;
SdVolume volume;
SdFile root;

DHT_Unified dht1(5, DHTTYPE);
DHT_Unified dht2(6, DHTTYPE);
// seconds to delay after readings
#define READING_DELAY 10

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
sensors_event_t event;
sensor_t sensor;
uint16_t lcdWrites;
File dataFile;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  dht1.begin();
  dht2.begin();
  lcd.begin(20,4);
  Serial.print("\nInitializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println(F("Card failed, or not present"));
  }

  dataFile = SD.open(F("DATALOG.TXT"), FILE_WRITE);
  if (dataFile) {
    dataFile.print(F("Beginning write. Delay between readings: "));
    dataFile.print(READING_DELAY);
    dataFile.println(F(" seconds"));
    dataFile.close();
  } else {
    Serial.println(F("error opening DATALOG.TXT"));
    return;
  }

  lcd.setCursor(0,2);
  lcd.print(F("SD OK."));
  
  dht1.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Humidity"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht1.humidity().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Humidity"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println(F("------------------------------------"));
  dht1.humidity().getEvent(&event);
  lcd.setCursor(0,0); //Start at character 4 on line 0
  lcd.print(event.relative_humidity);

  dht2.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Humidity"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println(F("------------------------------------"));
//   Print humidity sensor details.
  dht2.humidity().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Humidity"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println(F("------------------------------------"));
  dht2.humidity().getEvent(&event);
  lcd.setCursor(0,1);
  lcd.print(event.relative_humidity);
  delay(3000);
}

void loop() {
  // put your main code here, to run repeatedly:
  lcd.clear();
  dht1.humidity().getEvent(&event);
  lcd.setCursor(0,0); //Start at character 4 on line 0
  lcd.print(event.relative_humidity);
  Serial.print(event.relative_humidity);
  Serial.print(F(", "));
  dataFile = SD.open(F("DATALOG.TXT"), FILE_WRITE);
  dataFile.print(event.relative_humidity);
  dataFile.print(F(", "));
  dht2.humidity().getEvent(&event);
  lcd.setCursor(0,1);
  lcd.print(event.relative_humidity);
  Serial.println(event.relative_humidity);
  dataFile.println(event.relative_humidity);
  lcd.setCursor(0,2);
  lcd.print(F("SD Writes: "));
  lcd.setCursor(20-5,2);
  lcd.print(lcdWrites++);
  dataFile.close();
  delay(READING_DELAY * 1000);
}
