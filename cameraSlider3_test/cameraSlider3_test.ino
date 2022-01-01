#define PWMPIN 3

void setup() {
  // initialize digital pin OUTLED as an output.
  Serial.begin(115200);
  pinMode(PWMPIN, OUTPUT);
  // set as outputs
  DDRB |= (1 << DDB6) | (1 << DDB7);
  TCCR1B = TCCR1B & B11111000 | B00000100;    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
}

int freeRam2() {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void loop() {
  Serial.print("Loop ");
  Serial.println(freeRam2());
  
  // set PB6 high
  PORTB |= (1 << PORTB6);
  delay(1000);

  for (int i = 0; i < 255; i++) {
    analogWrite(PWMPIN, i);
    delay(10);
    if (i%10 == 0) {
      Serial.print("1 ");
      Serial.println(i);
    }
  }
  for (int i = 255; i >- 0; i--) {
    analogWrite(PWMPIN, i);
    delay(10);
    if (i%10 == 0) {
      Serial.print("1 ");
      Serial.println(i);
    }
  }
  
  // set PB6 low
  PORTB &= ~(1 << PORTB6);
  
  // both high
  PORTB |= (1 << PORTB7);
  delay(1000);
  for (int i = 0; i < 255; i++) {
    analogWrite(PWMPIN, i);
    delay(10);
    if (i%10 == 0) {
      Serial.print("2 ");
      Serial.println(i);
    }
  }
  for (int i = 255; i >- 0; i--) {
    analogWrite(PWMPIN, i);
    delay(10);
    if (i%10 == 0) {
      Serial.print("2 ");
      Serial.println(i);
    }
  }
  // both low
  PORTB &= ~(1 << PORTB7);
  delay(1000);

  
}
