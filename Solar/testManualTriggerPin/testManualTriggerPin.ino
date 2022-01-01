#define manualTriggerPin 2

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(manualTriggerPin, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("PinReading: ");
  Serial.println(digitalRead(manualTriggerPin));
  delay(200);
}
