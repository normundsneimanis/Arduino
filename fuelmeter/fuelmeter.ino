/* projekts
 *  Salodeet 2 kraasu ledu, 2 rezistorus, ground
 *  Nopirkt vienu 9v batereju, konektoru
 *  iesleegshanas pogu
 *  
 */

#include "timedelay.h"

#define FUELPIN 0
// background requires PWM because LED works from 3.3V - 1
#define BACKGROUND 1
#define PINRED 2
#define PINGREEN 4
//#define RESETPIN 3

timedelay fueldelay(10000);
timedelay ledDelay(2000);
timedelay bgDelay(2000);
timedelay resetDelay(120000L);

byte fuelState = 0; // states: 0 - 0; 1 - half, 2 - full;
byte ledNum = 0; // 0 = led1, 1 - led2
byte bgStatus = 1; // 1 - on, 0 - off

void setup() {
  // Set good pwm frequency for our LED diode
  //TCCR0B = (TCCR0B & (0b11111000 | 0b001));
  pinMode(FUELPIN, OUTPUT);
  pinMode(PINRED, OUTPUT);
  pinMode(PINGREEN, OUTPUT);
  pinMode(BACKGROUND, OUTPUT);
//  pinMode(RESETPIN, OUTPUT);
  randomSeed(analogRead(0));
//  Serial.begin(115200);
}

void loop() {
  bgDelay.cycle();
  fueldelay.cycle();
  ledDelay.cycle();
//  resetDelay.cycle();
//  if (resetDelay.hasPassed()) {
//    digitalWrite(RESETPIN, HIGH);
//  }
  if (fueldelay.hasPassed()) {
    analogWrite(FUELPIN, random(240));
    fueldelay.setTime(random(500, 4000));
    fueldelay.reset();
  }

  if (ledDelay.hasPassed()) {
    if (ledNum) {
      digitalWrite(PINRED, HIGH);
      digitalWrite(PINGREEN, LOW);
      ledNum = 0;
      ledDelay.setTime(random(200));
      ledDelay.reset();
    } else {
      digitalWrite(PINRED, LOW);
      digitalWrite(PINGREEN, HIGH);
      ledNum = 1;
      if (random(10) <= 3) {
        ledDelay.setTime(random(2000));
      } else {
        ledDelay.setTime(random(300));
      }
      ledDelay.reset();
    }
  }

  if (bgDelay.hasPassed()) {
//    Serial.println(bgDelay.getTime());
    if (bgStatus) {
      bgStatus = 0;
      analogWrite(BACKGROUND, 0);
//      Serial.println("BG off");
      bgDelay.setTime(50);
    } else {
      bgStatus = 1;
      analogWrite(BACKGROUND, 200);
//      Serial.println("BG on");
      if (random(10) <= 2) {
        bgDelay.setTime(random(2000));
      } else {
         bgDelay.setTime(random(300));
      }
    }
    bgDelay.reset();
  }
}
