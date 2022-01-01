static int clockPin = 3;      // Control 7-segment LED 1st digit
static int clockPin2 = 4;     // Control 7-segment LED 2nd digit
static int clockResetPin = 5; // Reset both 7-segment LED displays to zero
int ledValue = 0;


/*
 * Sends a clock pulse to the counter making it advance.
 */
void clock(int pin) {
  digitalWrite(pin,HIGH);
  digitalWrite(pin,LOW);
}

void resetLed(void) {
  digitalWrite(clockResetPin,HIGH);
  digitalWrite(clockResetPin,LOW);
}

void setLed(int number) {
  static int previousNumber = -1;
  if (number > 99) {
    number = 99;
  }
  if (number == previousNumber) {
    return;
  }
  previousNumber = number;
  int secondDigit = (number % 10);
  int firstDigit = (number / 10);
  resetLed();
  while(secondDigit > 0 || firstDigit > 0) {
    if (secondDigit-- > 0) {
      clock(clockPin2);
    }
    if (firstDigit-- > 0) {
      clock(clockPin);
    }
  }
}


void setup() {
  Serial.begin(9600);
  pinMode(clockPin,OUTPUT);
  pinMode(clockPin2,OUTPUT);
  pinMode(clockResetPin,OUTPUT);
}

void loop() {
  int addValue = 1;
  while(1) {
    setLed(ledValue);
    
    ledValue += addValue;
    delay(50);
    if (ledValue >= 100) {
      ledValue = 99;
      setLed(ledValue);
      delay(500);
      addValue = -1;
    }
    if (ledValue <= 0) {
      ledValue = 0;
      setLed(ledValue);
      delay(500);
      break;
    }
  }
  

}
