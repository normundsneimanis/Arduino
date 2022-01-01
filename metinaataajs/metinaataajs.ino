#define ledRed 5
#define ledGreen 4
#define inputButton 8
#define mosfetOutput 11

void setup() {
  // put your setup code here, to run once:
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(inputButton, INPUT);
  digitalWrite(inputButton, HIGH);
  pinMode(mosfetOutput, OUTPUT);
  digitalWrite(ledGreen, HIGH);
  delay(50);
  digitalWrite(ledGreen, LOW);
  delay(50);
  digitalWrite(ledGreen, HIGH);
  delay(50);
  digitalWrite(ledGreen, LOW);
  delay(50);
  digitalWrite(ledGreen, HIGH);
  delay(50);
  digitalWrite(ledGreen, LOW);
}

uint8_t weldRequested = 0;
void loop() {
  // put your main code here, to run repeatedly:
  if ((!weldRequested) && (!digitalRead(inputButton))) {
    weldRequested = 1;
  }

  if (weldRequested) {
    digitalWrite(ledGreen, HIGH);
    delay(500);
    digitalWrite(ledGreen, LOW);
    delay(500);
    digitalWrite(ledGreen, HIGH);
    delay(500);
    digitalWrite(ledGreen, LOW);
    delay(500);
    digitalWrite(ledRed, HIGH);
    digitalWrite(mosfetOutput, HIGH);
    delay(1);
    digitalWrite(mosfetOutput, LOW);
    digitalWrite(ledRed, LOW);
    delay(1000);
    weldRequested = 0;
  } else {
    delay(50);
  }
}
