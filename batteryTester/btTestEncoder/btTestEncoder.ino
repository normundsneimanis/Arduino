#include "rotaryEncoder.h"

#define encoderPinLeft 5
#define encoderPinRight 6
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
  attachInterrupt(digitalPinToInterrupt(encoderPinRight), rotaryEncoderISR, CHANGE);
}

int lastReading = 0;
unsigned int iter = 0;
void loop() {
  if (encoder.getCurrentPosition() != lastReading) {
    lastReading = encoder.getCurrentPosition();
    Serial.println(lastReading);
  }
  if (encoder.buttonPressed()) {
    Serial.println(F("Button pressed"));
  }
  if (iter >= 20) {
    Serial.print(digitalRead(encoderPinLeft));
    Serial.print(" == ");
    Serial.println(digitalRead(encoderPinRight));
    iter = 0;
  }
  iter++;
  delay(50);
}
