/*
 * codeexample for using a 4051 * analog multiplexer / demultiplexer
 * by david c. and tomek n.* for k3
 *
 * edited by Ross R.
 * edited by Igor de Oliveira Sá.
 */  
unsigned char analogSelect = 0;
#define SERIALBUFFERSIZE 100

#define analogSelect0 2
#define analogSelect1 3
#define analogSelect2 4

void selectPort(unsigned int analogSelect) {
	digitalWrite(analogSelect0, bitRead(analogSelect, 0));
	digitalWrite(analogSelect1, bitRead(analogSelect, 1));
	digitalWrite(analogSelect2, bitRead(analogSelect, 2));
}

void setup(){
  pinMode(analogSelect0, OUTPUT);
  pinMode(analogSelect1, OUTPUT);
  pinMode(analogSelect2, OUTPUT);
  Serial.begin(115200);
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
    Serial.print("Reading port ");
    Serial.print(analogSelect);
    Serial.print(": ");
    reading = analogRead(A6);
    Serial.println(reading);
    delay(900);
  }
}



