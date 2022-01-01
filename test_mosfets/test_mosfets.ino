void setup() {
  // put your setup code here, to run once:
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
}

enum writeMode {
  INCREASING,
  DECREASING,
};

typedef writeMode writeMode;

int i = 0;
writeMode mode = INCREASING;

void loop() {
  // put your main code here, to run repeatedly:
  if (mode == INCREASING) {
    analogWrite(9, i++);
  } else {
    analogWrite(9, i--);
  }
  delay(20);
  if (i == 255) {
    mode = DECREASING;
  }
  if (i == 0) {
    mode = INCREASING;
    if (digitalRead(10) == 1) {
      digitalWrite(10, LOW);
    } else {
      digitalWrite(10, HIGH);
    }
  }
}
