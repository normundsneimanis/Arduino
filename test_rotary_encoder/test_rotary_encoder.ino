/* read a rotary encoder with interrupts
   Encoder hooked up with common to GROUND,
   encoder0PinA to pin 2, encoder0PinB to pin 4 (or pin 3 see below)
   it doesn't matter which encoder pin you use for A or B  

   uses Arduino pullups on A & B channel outputs
   turning on the pullups saves having to hook up resistors 
   to the A & B channel outputs 

*/

// Connect rotary encoder the following way
// where three pins: left and right goes to ports AB, middle is GND
// Button - one pin goes to GND, second to digital input

#define encoder0PinA  3
#define encoder0PinB  4
#define encoder0ButtonPin 7

volatile int encoder0Pos = 0;
volatile int encoderPosPrev = 0;
byte encoderStatus;
byte pinReading;

void setup() { 


  pinMode(encoder0PinA, INPUT); 
  digitalWrite(encoder0PinA, HIGH);       // turn on pullup resistor
  pinMode(encoder0PinB, INPUT); 
  digitalWrite(encoder0PinB, HIGH);       // turn on pullup resistor
  pinMode(encoder0ButtonPin, INPUT);
  digitalWrite(encoder0ButtonPin, HIGH);       // turn on pullup resistor

  attachInterrupt(digitalPinToInterrupt(encoder0PinA), doEncoder, CHANGE);  // encoder pin on interrupt 0 - pin 2
  Serial.begin (115200);
  Serial.println("start");                // a personal quirk

} 

void loop(){
   if (encoder0Pos != encoderPosPrev) {
    Serial.println(encoder0Pos, DEC);
    encoderPosPrev = encoder0Pos;
   }
   if (!digitalRead(encoder0ButtonPin)) {
    Serial.println(F("Button pressed"));
   }
   delay(50);
}

void doEncoder() {
  /* If pinA and pinB are both high or both low, it is spinning
   * forward. If they're different, it's going backward.
   *
   * For more information on speeding up this process, see
   * [Reference/PortManipulation], specifically the PIND register.
   */
  if (digitalRead(encoder0PinA) == digitalRead(encoder0PinB)) {
    encoder0Pos++;
  } else {
    encoder0Pos--;
  }
}
