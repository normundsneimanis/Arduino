
static int repeatPin = 6;     // (button) Pin to detect if movement should be repeated
static int timeLapsePin = 7;  // if ON, do time lapse, very slow movement
static int motorDirectionPin1 = 8;  // Input on which direction to go
static int motorDirectionPin2 = 9;  // Input on which direction to go

int dirPin1;
int dirPin2;
int repeatReading;
int timeLapseReading;
int previousPin1 = -1; // 1 = left, 2 - right
int previousPin2 = -1; // 1 = left, 2 - right
int previousRepeat = -1;
int previousTimeLapse = -1;

void setup() {
  Serial.begin(9600);
  pinMode(motorDirectionPin1, INPUT);
  pinMode(motorDirectionPin2, INPUT);
  pinMode(repeatPin, INPUT);
  pinMode(timeLapsePin, INPUT);
}

void loop() {
  dirPin1 = digitalRead(motorDirectionPin1);
  dirPin2 = digitalRead(motorDirectionPin2);
  repeatReading = digitalRead(repeatPin);
  timeLapseReading = digitalRead(timeLapsePin);
  if (previousPin1 != dirPin1 || previousPin2 != dirPin2 || repeatReading != previousRepeat || timeLapseReading != previousTimeLapse) {
    Serial.print("Repeat: ");
    Serial.print(repeatReading);
    Serial.print(" was: ");
    Serial.print(previousRepeat);

    Serial.print(" timeLapse: ");
    Serial.print(timeLapseReading);
    Serial.print(" was: ");
    Serial.print(previousTimeLapse);
    
    Serial.print(" dirPin1: ");
    Serial.print(dirPin1);
    Serial.print(" was: ");
    Serial.print(previousPin1);
    
    Serial.print(" dirPin2: ");
    Serial.print(dirPin2);
    Serial.print(" was: ");
    Serial.println(previousPin2);
    previousPin1 = dirPin1;
    previousPin2 = dirPin2;
    previousRepeat = repeatReading;
    previousTimeLapse = timeLapseReading;
  }
}
