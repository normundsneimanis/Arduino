#define statusLed 13
#define testLed 3

void setup() {
  // put your setup code here, to run once:
  pinMode(testLed, OUTPUT);
  pinMode(statusLed, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(9000);
  digitalWrite(statusLed, HIGH);
  delay(1000);
  digitalWrite(statusLed, LOW);
  digitalWrite(testLed, HIGH);
  delay(200);
  digitalWrite(testLed, LOW);
}
