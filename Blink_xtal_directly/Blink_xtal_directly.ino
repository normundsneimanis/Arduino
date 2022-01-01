/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin OUTLED. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://www.arduino.cc

  This example code is in the public domain.

  modified 13 May 2014
  by Scott Fitzgerald
 */
//static int motor1Switch = A6;
//static int motor2Switch = A7;

void setup() {
  // initialize digital pin OUTLED as an output.
  Serial.begin(115200);
  // set as outputs
  DDRB |= (1 << DDB6) | (1 << DDB7);
}

// Blinks LED OUTLED 5 times period that is given as argument
// Arguments: int sleep - microseconds to sleep between blinks
//void Blink(int sleep) {
//  int iterator;
//  for (iterator = 0; iterator < 5; iterator++) {
//    digitalWrite(OUTLED, HIGH);
//    delay(sleep);
//    digitalWrite(OUTLED, LOW);
//    delay(sleep);
//  }
//}

void loop() {
  // set PB6 high
  PORTB |= (1 << PORTB6);
  delay(1000);
  // set PB6 low
  PORTB &= ~(1 << PORTB6);
  delay(1000);
  
  // both high
  PORTB |= (1 << PORTB7);
  delay(1000);
  // both low
  PORTB &= ~(1 << PORTB7);
  delay(1000);
}
