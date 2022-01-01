#include <stdint.h>
#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address


#define sensorPort A3
#define powerPort 2
#define motorPort 3
#define readyPort 4

#define GRACETIME 60 * 60
#define STARTHUMIDITY 80
#define ENDHUMIDITY 70
// keep motor on for maximum one hour
#define MAXMOTOR 60 * 60

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  lcd.begin(20,4);   // initialize the lcd for 16 chars 2 lines, turn on backlight
  pinMode(powerPort, OUTPUT);
  pinMode(sensorPort, INPUT);
  pinMode(motorPort, OUTPUT);
  pinMode(readyPort, OUTPUT);
  lcd.setCursor(0,0); //Start at character 4 on line 0
  lcd.print("Humidity: ");
  digitalWrite(readyPort, HIGH);
}

// 30% = 900k, voltage = 0.271
// 40% = 270k, voltage = 0.802
// 50% = 81k, voltage = 1.908
// 60% = 33k, voltage = 3.012
// 70% = 13k 3.968
// 80% = 5k3 4.521
// 90% = 2k2 4.789
// 95% = 1k5 4.854

int reading;
int sensorReading;
uint8_t motorStarted = 0;
uint16_t graceTime = 0;
uint8_t started = 0;
uint16_t timeOn = 0;
void loop() {
  Serial.print("started: ");
  Serial.print(started);
  Serial.print(" timeOn: ");
  Serial.print(timeOn);
  Serial.print(" graceTime: ");
  Serial.print(graceTime);
  digitalWrite(powerPort, HIGH);
  sensorReading = analogRead(sensorPort);
  reading = map(sensorReading, 0, 1024, 0, 500);
  Serial.print(" voltage: ");
  Serial.print(reading);
  Serial.print(" Humidity: ");
  if (reading <= 27) {
    reading = 30;
  } else if (reading > 27 && reading <= 80) {
    reading = map(reading, 27, 80, 30, 40);
  } else if (reading > 80 && reading <= 191) {
    reading = map(reading, 80, 191, 40, 50);
  } else if (reading > 191 && reading < 301) {
    reading = map(reading, 191, 301, 50, 60);
  } else if (reading >= 301 && reading < 397) {
    reading = map(reading, 301, 397, 60, 70);
  } else if (reading >= 397 && reading < 452) {
    reading = map(reading, 397, 452, 70, 80);
  } else if (reading >= 452 && reading < 479) {
    reading = map(reading, 452, 479, 80, 90);
  } else {
    reading = 95;
  }
  
  lcd.setCursor(10,0);
  lcd.print("   ");
  lcd.setCursor(10,0);
  lcd.print(reading);
  lcd.print('%');
  Serial.print(reading);
  Serial.println("% RH");
  digitalWrite(powerPort, LOW);
  
  if ((!started) && (reading > STARTHUMIDITY && graceTime == 0)) {
    started = 1;
    digitalWrite(motorPort, HIGH);
    timeOn = 0;
    digitalWrite(readyPort, LOW);
    graceTime = 1;
  }

  if (started) {
    lcd.setCursor(0,1);
    lcd.print("Time: ");
    lcd.print(timeOn / 60);
    lcd.print(":");
    lcd.print(timeOn % 60);
    timeOn++;
  } else {
    if (graceTime) {
      graceTime--;
    } else {
      digitalWrite(readyPort, HIGH);
    }
  }

  if ((started) && (reading < ENDHUMIDITY || timeOn > MAXMOTOR)) {
    started = 0;
    digitalWrite(motorPort, LOW);
    graceTime = GRACETIME;
    graceTime = 60;
    digitalWrite(readyPort, LOW);
  }
  
  delay(1000);
}
