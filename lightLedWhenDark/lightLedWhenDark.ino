int sensorPin = A1;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
   pinMode(ledPin, OUTPUT);
   Serial.begin(9600);
}

void loop() {
  sensorValue = analogRead(sensorPin);
//  Serial.print("Received the follwing value: [");
//  Serial.print(sensorValue);
//  Serial.print("]\n");
  if (sensorValue > 100)
    digitalWrite(ledPin, HIGH);
  else if ((sensorValue < 100))
    digitalWrite(ledPin, LOW);
  delay(20);
}
