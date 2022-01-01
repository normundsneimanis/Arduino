static int pwmPin = 14;
static int dir1 = 13;
static int dir2 = 12;
int motorPinCurrent;
int motorPinOff;
int motorValue = 0;

volatile uint8_t printFlag = 0;
ISR(TIMER1_COMPA_vect)
{
  printFlag++;
}

void setup() {
  Serial.begin(115200);
  TCCR1A = 0;
  TCCR1B = 0; // Stop timer so we can modify it
  TCCR1B |= (1 << WGM12); // Configure timer 1 for CTC mode
  OCR1A   = 31250; // Set CTC compare value to 1Hz at 1MHz AVR clock, with a prescaler of 64
  TIMSK1 |= (1 << OCIE1A); // Enable CTC interrupt
  sei(); //  Enable global interrupts
  TCCR1B |= ((1 << CS12)); // Set up timer at Fcpu/256
//  TCCR1B = TCCR1B & B11111000 | B00000100;    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
  pinMode(pwmPin, OUTPUT);
  pinMode(dir1, OUTPUT);
  pinMode(dir2, OUTPUT);
//  DDRB |= (1 << DDB6) | (1 << DDB7);
}


uint16_t counter = 0;
void loop() {
  cli();
  while (printFlag > 0) {
    cli();
    printFlag--;
    sei();
    Serial.print("Interrupts: ");
    Serial.println(counter);
    counter++;
  }
  sei();
  // set PB6 high
//  PORTB |= (1 << PORTB6);
  digitalWrite(dir1, HIGH);
  for (int i = 0; i < 255; i+=5) {
    analogWrite(pwmPin, i);
    Serial.println(i);
    delay(50);
  }
  delay(1000);
  for (int i = 255; i > 0; i-=5) {
    analogWrite(pwmPin, i);
    Serial.println(i);
    delay(50);
  }
  // set PB6 low
//  PORTB &= ~(1 << PORTB6);
  digitalWrite(dir1, LOW);
  delay(1000);
  
  // B7 high
//  PORTB |= (1 << PORTB7);
  digitalWrite(dir2, HIGH);
  for (int i = 0; i < 255; i+=5) {
    analogWrite(pwmPin, i);
    Serial.println(i);
    delay(50);
  }
  delay(1000);
  for (int i = 255; i > 0; i-=5) {
    analogWrite(pwmPin, i);
    Serial.println(i);
    delay(50);
  }
    // both low
//  PORTB &= ~(1 << PORTB7);
  digitalWrite(dir2, LOW);
  delay(1000);
}
