#include "timeDelay.h"

#define sensorPin 3
#define statusLed 13
#define SIGNALS_MAX 32

enum irStats {
  interruptCalled,
  NUM_STATS,
};

enum irState {
  START,
  ZERO,
  ONE,
};

typedef irStats irStats;

unsigned long signals[SIGNALS_MAX];
int signalsIndex = 0;
int signalsPrinted = 0; // 0 - not printed, 1 - start printing, 2 - printed
byte portState = 0;
unsigned long pulseStartTime;
unsigned long stats[NUM_STATS];
unsigned long pulseTime;

timedelay statsDelay(1000);

void doEncoder() {
  stats[interruptCalled]++;
  if (digitalRead(sensorPin) == HIGH) {
    portState = 1;
    pulseStartTime = micros();
  } else {
    portState = 0;
    // 4488 - 4500 = start
    // 1664 - 1700 = high
    // 568 - 576 = low
//    pulseTime = micros() - pulseStartTime;
//    if (pulseTime >= 4480 && pulseTime <= 4500) {
//      signalsIndex = 0;
//      signals[signalsIndex] = 0;
//    } else if (pulseTime >= 1660 && pulseTime <= 1700) {
//      signals[signalsIndex] = 1;
//    } else if (pulseTime >= 540 && pulseTime <= 580) {
//      signals[signalsIndex] = 2;
//    } else {
//      signalsIndex = 0;
//    }
    signals[signalsIndex] = micros() - pulseStartTime;
    //signals[signalsIndex] = pulseIn(sensorPin,LOW);
    if (signalsPrinted == 0) {
      signalsIndex++;
    }
  
    // reset index only if contents have been printed
    if (signalsIndex == SIGNALS_MAX && signalsPrinted == 0) {
      signalsPrinted = 1;
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  pinMode(statusLed, OUTPUT);
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);       // turn on pullup resistor
  Serial.begin(115200);
  stats[interruptCalled] = 0;
  attachInterrupt(digitalPinToInterrupt(sensorPin), doEncoder, CHANGE);
  digitalWrite(statusLed, HIGH);
  delay(10);
  digitalWrite(statusLed, LOW);
  delay(50);
  digitalWrite(statusLed, HIGH);
  delay(10);
  digitalWrite(statusLed, LOW);
  delay(50);
  digitalWrite(statusLed, HIGH);
  delay(10);
  digitalWrite(statusLed, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  statsDelay.cycle();
  if (statsDelay.hasPassed()) {
    statsDelay.reset();
    Serial.print("Signal increased ");
    Serial.print(signalsIndex);
    Serial.print(" times. Interrupt called: ");
    Serial.print(stats[interruptCalled]);
    Serial.println(" times.");
  }

  if (signalsPrinted == 1) {
    digitalWrite(statusLed, HIGH);
    delay(10);
    digitalWrite(statusLed, LOW);
    Serial.println("Array contents (timings): ");
    for (int i = 0; i < SIGNALS_MAX; i++) {
      Serial.print(i);
      Serial.print(":\t");
      Serial.println(signals[i]);
    }
    signalsPrinted = 0;
    signalsIndex = 0;
  }

}
