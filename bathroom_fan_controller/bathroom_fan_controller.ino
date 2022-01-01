#include <stdint.h>
//#include <Wire.h>  // Comes with Arduino IDE
//#include <LiquidCrystal_I2C.h>
//LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address


//#define sensorPort A3
//#define powerPort 2
//#define motorPort 3
//#define readyPort 4

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN            5         // Pin which is connected to the DHT sensor.
#define DHTTYPE           DHT11     // DHT 11

#define ledGreen A0
#define ledYellow A1
#define ledRed 13

#define manualTrigger A2
#define input1 6
#define input2 7
#define input3 8
#define input4 9
#define input5 10
#define input6 11
/*
 * Six configuration input pin settings
 * 1. max working time, 30, 60, 120, 240 - 2 pins
 * 2. max delay time, 0, 30, 60, 120 - 2 pins
 * 3. threshold - 90%, 80%, 1 pin
 * 4. off threshold - threshold - 10%, threshold - 20%, 1 pin
 */

DHT_Unified dht(DHTPIN, DHTTYPE);

/*
 * Software workflow
 * 
 * read configuration settings
 * if trigger requested
 *  if enabled - disable, set delay time to 30 minutes
 *  if disabled - enable for 30 minutes
 * 
 * if delay time passed and humidity reached
 *  enable for configured time
 * 
 * if enabled and off humidity threshold reached
 *  disable
 *  set delay time to configured value
 * 
 * if enabled and configured time passed
 *  disable
 *  set delay time to configured value
 *  
 * 
 */

// How long should fan work after manual trigger
#define TRIGGER_TIME 15

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  DDRB |= (1 << DDB6);
  
//  lcd.begin(20,4);   // initialize the lcd for 16 chars 2 lines, turn on backlight
  pinMode(manualTrigger, INPUT);
  digitalWrite(manualTrigger, HIGH);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledYellow, OUTPUT);

  pinMode(input1, INPUT);
  pinMode(input2, INPUT);
  pinMode(input3, INPUT);
  pinMode(input4, INPUT);
  pinMode(input5, INPUT);
  pinMode(input6, INPUT);
  digitalWrite(input1, HIGH);
  digitalWrite(input2, HIGH);
  digitalWrite(input3, HIGH);
  digitalWrite(input4, HIGH);
  digitalWrite(input5, HIGH);
  digitalWrite(input6, HIGH);
  
//  lcd.setCursor(0,0); //Start at character 4 on line 0
//  lcd.print("Humidity: ");
//  digitalWrite(readyPort, HIGH);

  digitalWrite(ledYellow, HIGH);
  delay(50);
  digitalWrite(ledYellow, LOW);
  digitalWrite(ledGreen, HIGH);
  delay(50);
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledRed, HIGH);
  delay(50);
  digitalWrite(ledRed, LOW);

  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
  Serial.println("------------------------------------");
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println("------------------------------------");

  readConfig();
  readHumidity();
  startISR();
}

uint16_t configRunTime = 0;
uint16_t configWaitTime = 0;
uint8_t configThreshold = 0;
uint8_t configOffThreshold = 0;
void readConfig() {
  Serial.print("Input switches status: ");
  Serial.print(!digitalRead(input1));
  Serial.print(!digitalRead(input2));
  Serial.print(!digitalRead(input3));
  Serial.print(!digitalRead(input4));
  Serial.print(!digitalRead(input5));
  Serial.println(!digitalRead(input6));

  // max run time in minutes
  if (digitalRead(input1) && digitalRead(input2)) {
    configRunTime = 15; // 30
  } else if (!digitalRead(input1) && digitalRead(input2)) {
    configRunTime = 30; // 60
  } else if (digitalRead(input1) && !digitalRead(input2)) {
    configRunTime = 60; // 120 
  } else if (!digitalRead(input1) && !digitalRead(input2)) {
    configRunTime = 120; // 240
  }

  // min wait time in minutes after run
  if (digitalRead(input3) && digitalRead(input4)) {
    configWaitTime = 0; // 0
  } else if (!digitalRead(input3) && digitalRead(input4)) {
    configWaitTime = 15; // 30
  } else if (digitalRead(input3) && !digitalRead(input4)) {
    configWaitTime = 30; // 60
  } else if (!digitalRead(input3) && !digitalRead(input4)) {
    configWaitTime = 60; // 120
  }

  if (!digitalRead(input5)) {
    configThreshold = 80; // 80
  } else {
    configThreshold = 70; // 70 
  }

  if (!digitalRead(input6)) {
    configOffThreshold = configThreshold - 10; //10
  } else {
    configOffThreshold = configThreshold - 20; //20
  }

  Serial.print("Config. RunTime: ");
  Serial.print(configRunTime);
  Serial.print(". WaitTime: ");
  Serial.print(configWaitTime);
  Serial.print(". Threshold: ");
  Serial.print(configThreshold);
  Serial.print(". OffThrehsold: ");
  Serial.println(configOffThreshold);
}

uint8_t motorEnabled = 0;
volatile uint32_t runTimeLeft = 0;
volatile uint32_t waitTimeLeft = 0;
uint8_t manuallyTriggerred = 0;

void motorDisable() {
  PORTB &= ~(1 << PORTB6);
  digitalWrite(ledRed, LOW);
  motorEnabled = 0;
  runTimeLeft = 0;
  manuallyTriggerred = 0;
  waitTimeLeft = configWaitTime * 60;
}

void motorEnable() {
  digitalWrite(ledRed, HIGH);
  PORTB |= (1 << PORTB6);
  motorEnabled = 1;
  runTimeLeft = configRunTime * 60;
//  runTimeLeft = 60; // testing
  waitTimeLeft = 0;
}

sensors_event_t event;

volatile uint32_t interruptCounter = 0;
volatile uint8_t buttonPressed = 0;
volatile uint8_t triggerHumidityRead = 0;
volatile uint8_t triggerPrint = 0;
volatile uint32_t motorLastTriggerred = 0;
volatile uint8_t yellowLedEnabled = 0;
ISR(TIMER1_COMPA_vect) {
  interruptCounter++;
  
  if (yellowLedEnabled) {
    digitalWrite(ledYellow, LOW);
    yellowLedEnabled = 0;
  }
  
  if (interruptCounter % 10 == 0) {
    if (waitTimeLeft) {
      waitTimeLeft--;
    }
    if (runTimeLeft) {
      runTimeLeft--;
    }
    triggerPrint = 1;
  }

  if (interruptCounter % 50 == 0) {
    if ((!motorEnabled && (int) event.relative_humidity >= configThreshold) || 
    (motorEnabled && (int) event.relative_humidity > configOffThreshold))
    digitalWrite(ledYellow, HIGH);
    yellowLedEnabled = 1;
  }

  if (interruptCounter % 100 == 0) {
    triggerHumidityRead = 1;
  }

  if (motorLastTriggerred + 10 < interruptCounter && !digitalRead(manualTrigger)) {
    buttonPressed = 1;
    motorLastTriggerred = interruptCounter;
  }

  if (interruptCounter < motorLastTriggerred) {
    motorLastTriggerred = 0;
  }
}

void startISR() {
  cli();
  TCCR1A = 0;
  TCCR1B = 0; // Stop timer so we can modify it
  TCCR1B |= _BV(WGM12);; // Configure timer 1 for CTC mode
  OCR1A   = 3125 - 1; // Set CTC compare value to 10Hz at 8MHz AVR clock, with a prescaler of 128
  TCCR1B |= (1 << CS12); // Set up timer at Fcpu/128
  TIMSK1 |= (1 << OCIE1A);// | (1 << OCIE1B); // Enable CTC interrupt
  sei(); //  Enable global interrupts
}

void readHumidity() {
  digitalWrite(ledGreen, HIGH);
  dht.humidity().getEvent(&event);
  digitalWrite(ledGreen, LOW);
}

void loop() {
  if (buttonPressed) {
    cli();
    buttonPressed = 0;
    sei();
    if (!motorEnabled) {
      Serial.println("Trigger enabling motor");
      motorEnable();
      runTimeLeft = TRIGGER_TIME * 60;
      manuallyTriggerred = 1;
    }
  }

  if (triggerHumidityRead) {
    cli();
    triggerHumidityRead = 0;
    sei();
    readHumidity();
  }
  
  if (!motorEnabled && !waitTimeLeft && (int) event.relative_humidity >= configThreshold) {
    motorEnable();
    Serial.print("Enabling Motor ");
    Serial.println((int) event.relative_humidity);
  }

  if (motorEnabled) {
    if (!manuallyTriggerred && (!runTimeLeft || (int) event.relative_humidity < configOffThreshold)) {
      motorDisable();
      Serial.println("Disabling Motor");
    }
    if (manuallyTriggerred && !runTimeLeft) {
      motorDisable();
      Serial.println("Disabling manually enabled motor");
    }
  }

  if (triggerPrint) {
    cli();
    triggerPrint = 0;
    sei();
    Serial.print("motorEnabled: ");
    Serial.print(motorEnabled);
    Serial.print(" Humidity: ");
    Serial.print((int) event.relative_humidity);
    Serial.print("% Time: ");
    Serial.print(interruptCounter / 10);
    Serial.print(" waitTimeLeft: ");
    Serial.print(waitTimeLeft);
    Serial.print(" runTimeLeft: ");
    Serial.println(runTimeLeft);
  }
}


// 30% = 900k, voltage = 0.271
// 40% = 270k, voltage = 0.802
// 50% = 81k, voltage = 1.908
// 60% = 33k, voltage = 3.012
// 70% = 13k 3.968
// 80% = 5k3 4.521
// 90% = 2k2 4.789
// 95% = 1k5 4.854

//int reading;
//int sensorReading;
//uint8_t motorStarted = 0;
//uint16_t graceTime = 0;
//uint8_t started = 0;
//uint16_t timeOn = 0;
//uint8_t enabled = 0;
//
//sensors_event_t event;
//void loop() {
//  digitalWrite(ledYellow, HIGH);
//  Serial.print(!digitalRead(manualTrigger));
//  Serial.print(!digitalRead(input1));
//  Serial.print(!digitalRead(input2));
//  Serial.print(!digitalRead(input3));
//  Serial.print(!digitalRead(input4));
//  Serial.print(!digitalRead(input5));
//  Serial.print(!digitalRead(input6));
//  Serial.print(" started: ");
//  Serial.print(started);
//  Serial.print(" timeOn: ");
//  Serial.print(timeOn);
//  Serial.print(" graceTime: ");
//  Serial.print(graceTime);
//  if (enabled) {
//    PORTB &= ~(1 << PORTB6);
//    enabled--;
//    digitalWrite(ledRed, LOW);
//  } else {
//    enabled = 5;
//    digitalWrite(ledRed, HIGH);
//    PORTB |= (1 << PORTB6);
//  }
////  digitalWrite(powerPort, HIGH);
////  sensorReading = analogRead(sensorPort);
////  reading = map(sensorReading, 0, 1024, 0, 500);
////  Serial.print(" voltage: ");
////  Serial.print(reading);
////  Serial.print(" Humidity: ");
////  if (reading <= 27) {
////    reading = 30;
////  } else if (reading > 27 && reading <= 80) {
////    reading = map(reading, 27, 80, 30, 40);
////  } else if (reading > 80 && reading <= 191) {
////    reading = map(reading, 80, 191, 40, 50);
////  } else if (reading > 191 && reading < 301) {
////    reading = map(reading, 191, 301, 50, 60);
////  } else if (reading >= 301 && reading < 397) {
////    reading = map(reading, 301, 397, 60, 70);
////  } else if (reading >= 397 && reading < 452) {
////    reading = map(reading, 397, 452, 70, 80);
////  } else if (reading >= 452 && reading < 479) {
////    reading = map(reading, 452, 479, 80, 90);
////  } else {
////    reading = 95;
////  }
//  
////  lcd.setCursor(10,0);
////  lcd.print("   ");
////  lcd.setCursor(10,0);
////  lcd.print(reading);
////  lcd.print('%');
////  Serial.print(reading);
////  Serial.println("% RH");
////  digitalWrite(powerPort, LOW);
//
//  // Get humidity event and print its value.
//  dht.humidity().getEvent(&event);
//  if (isnan(event.relative_humidity)) {
//    Serial.println("Error reading humidity!");
//  }
//  else {
//    Serial.print(" Humidity: ");
//    Serial.print(event.relative_humidity);
//    Serial.println("%");
//  }
//  
////  if ((!started) && (reading > STARTHUMIDITY && graceTime == 0)) {
////    started = 1;
////    digitalWrite(motorPort, HIGH);
////    timeOn = 0;
////    digitalWrite(readyPort, LOW);
////    graceTime = 1;
////  }
////
////  if (started) {
////    lcd.setCursor(0,1);
////    lcd.print("Time: ");
////    lcd.print(timeOn / 60);
////    lcd.print(":");
////    lcd.print(timeOn % 60);
////    timeOn++;
////  } else {
////    if (graceTime) {
////      graceTime--;
////    } else {
////      digitalWrite(readyPort, HIGH);
////    }
////  }
////
////  if ((started) && (reading < ENDHUMIDITY || timeOn > MAXMOTOR)) {
////    started = 0;
////    digitalWrite(motorPort, LOW);
////    graceTime = GRACETIME;
////    graceTime = 60;
////    digitalWrite(readyPort, LOW);
////  }
//  digitalWrite(ledYellow, LOW);
//  delay(2000);
//}
