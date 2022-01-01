#define ASCII_ONE 49
#define ASCII_ZERO 48

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {
    int bval = Serial.read();
    Serial.print("Received the follwing value: [");
    Serial.write(bval);
    Serial.print("]\n");
    if (bval == ASCII_ONE) {
      digitalWrite(13, HIGH);
    } else if (bval == ASCII_ZERO) {
      digitalWrite(13, LOW);
    }
  }
}
