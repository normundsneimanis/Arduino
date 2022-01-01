static int hallSensor1 = 15;
static int hallSensor2 = 16;
int hallSensor1Value;
int hallSensor2Value;
int old1 = -1;
int old2 = -1;

void setup() {
  Serial.begin(9600);
  pinMode(hallSensor1, INPUT);
  pinMode(hallSensor2, INPUT);
}

void loop() {
  hallSensor1Value = analogRead(hallSensor1);
  hallSensor2Value = analogRead(hallSensor2);
  if (hallSensor1Value != old1 || hallSensor2Value != old2) {
    Serial.print(" 1: ");
    Serial.print(hallSensor1Value);
    Serial.print(" was: ");
    Serial.print(old1);
    Serial.print("\t2: ");
    Serial.print(hallSensor2Value);
    Serial.print(" was: ");
    Serial.println(old2);
    old1 = hallSensor1Value;
    old2 = hallSensor2Value;
  }
  delay(100);
}
