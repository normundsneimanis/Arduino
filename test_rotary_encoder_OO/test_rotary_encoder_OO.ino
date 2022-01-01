#include "rotaryEncoder.h"

#define encoderPinLeft 3
#define encoderPinRight 4
#define encoder0ButtonPin 7

rotaryEncoder encoder(encoderPinLeft, encoderPinRight, encoder0ButtonPin);

void rotaryEncoderISR() {
  if (digitalRead(encoderPinLeft) == digitalRead(encoderPinRight)) {
    encoder.stepUp();
  } else {
    encoder.stepDown();
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("start");
  encoder.init();
  encoder.setStep(5);
  attachInterrupt(digitalPinToInterrupt(encoderPinLeft), rotaryEncoderISR, CHANGE);
}

int lastReading = 0;

void loop() {
  if (encoder.getCurrentPosition() != lastReading) {
    lastReading = encoder.getCurrentPosition();
    Serial.println(lastReading);
  }
  if (encoder.buttonPressed()) {
    Serial.println(F("Button pressed"));
  }
  delay(50);
}
