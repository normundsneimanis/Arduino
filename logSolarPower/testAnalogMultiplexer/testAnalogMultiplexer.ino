/*
 * codeexample for using a 4051 * analog multiplexer / demultiplexer
 * by david c. and tomek n.* for k3
 *
 * edited by Ross R.
 * edited by Igor de Oliveira Sá.
 */  
unsigned char analogSelect = 0;
#define SERIALBUFFERSIZE 100

#define analogSelect0 15
#define analogSelect1 18
#define analogSelect2 19

#define enableHeatSensorsPin 4

void selectPort(unsigned int analogSelect) {
  digitalWrite(analogSelect0, bitRead(analogSelect, 0));
  digitalWrite(analogSelect1, bitRead(analogSelect, 1));
  digitalWrite(analogSelect2, bitRead(analogSelect, 2));
}

int voltageDivider(int Vin, int R1, int R2) {
  // calculate voltage divider and convert to Arduino input value
  // Vout = ((Vin * R2) / (R1 + R2))
//  Serial.print("Input: ");
//  Serial.print(R1);
//  Serial.print(" Divider: ");
//  Serial.print((((float) Vin * R2) / ((float) R1 + R2)));
//  Serial.print(" multiplier: ");
//  Serial.print((float) 5.0 / 1024.0, 6);
//  Serial.print(" result(float): ");
//  Serial.print(((float) ((Vin * R2) / (R1 + R2)) / ((float) 5.0 / 1024.0)), 2);
//  Serial.print(" Divider result: ");
//  Serial.println((int) ((float) (((float) Vin * R2) / (R1 + R2)) / ((float) 5.0 / 1024.0) + 0.5));
  return (int) ((float) (((float) Vin * R2) / (R1 + R2)) / ((float) 5.0 / 1024.0) + 0.5);
}

void setup(){
  pinMode(analogSelect0, OUTPUT);
  pinMode(analogSelect1, OUTPUT);
  pinMode(analogSelect2, OUTPUT);
  pinMode(enableHeatSensorsPin, OUTPUT);
  Serial.begin(115200);
  digitalWrite(enableHeatSensorsPin, HIGH);
}

float fmap (float sensorValue, float sensorMin, float sensorMax, float outMin, float outMax) {
	return (sensorValue - sensorMin) * (outMax - outMin) / (sensorMax - sensorMin) + outMin;
}

int reading;
char serialBuffer[SERIALBUFFERSIZE];
void loop () {
  unsigned int analogSelect = 0;
  int index = 0;
  memset(serialBuffer, 0, SERIALBUFFERSIZE);
  while (Serial.available()) {
    serialBuffer[index++] = Serial.read();
    delay(1); // Sleep so all serial input is being read in one buffer
  }
  analogSelect = atoi(serialBuffer);
  // select the bit  
  selectPort(analogSelect);
  // Read the analog input until another is requested via console
  while (!Serial.available()) {
    digitalWrite(enableHeatSensorsPin, HIGH);
    Serial.print("Reading port ");
    Serial.print(analogSelect);
    Serial.print(": ");
    reading = analogRead(A0);
    //digitalWrite(enableHeatSensorsPin, LOW);
    Serial.print(reading);
    Serial.print(" (");
    Serial.print(fmap((float) reading, 0.0, 1024.0, 0.0, 5.0));
    Serial.println(")");
    delay(900);
  }
}


