#define nightThreshold 200
#define photoResistorPin A0

void setup () {
  Serial.begin(115200);
  Serial.println("Initiating photoresistor measurement");
  pinMode(photoResistorPin, INPUT);
}

int rawLight;

void loop () {
  rawLight = analogRead(photoResistorPin);
  Serial.print("Analog value ");
  Serial.print(rawLight);
  if (rawLight < nightThreshold) {
    Serial.println(" Night");
  } else {
    Serial.println(" Day");
  }
  delay(1000);
}
