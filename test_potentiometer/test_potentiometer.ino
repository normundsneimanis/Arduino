static int sensorPin = 14;
int sensorValue;
int sensorValuePrev;
int motorValue;
static int sleeptime = 100;


void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
}

void loop() {
  sensorValue = ((analogRead(sensorPin) - 1020) * -1);

  Serial.print("RawPot: ");
  Serial.print(sensorValue);

  // We need numbers 0 - 100, but have 0 - 1024, so divide by 10
  // This needs to be calibrated after new potentiometer is used.
  sensorValue = sensorValue / 10;
  if (sensorValue < 0)
    sensorValue = 0;
  if (sensorValue > 99) {
    sensorValue = 99;
  }
  // We want to do little histeresys so our motor speed stays constant upon small sensor changes
  if (sensorValue < sensorValuePrev + 1 && sensorValue > sensorValuePrev - 1) {
    sensorValue = sensorValuePrev;
  } else {
    sensorValuePrev = sensorValue;
  }
  motorValue = sensorValue * 2.5758;

  Serial.print(" Pot: ");
  Serial.print(sensorValue);
  Serial.print(" PWM: ");
  Serial.println(motorValue);
  delay(sleeptime);
}
