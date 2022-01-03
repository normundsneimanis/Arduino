/*
 * Arduino TQFP32 pinout
 */
#define PWMCHOOSE 5
#define motorPinGo 3
#define pocPin A0

void setup() {
  // put your setup code here, to run once:
  pinMode(PWMCHOOSE, OUTPUT);
  pinMode(motorPinGo, OUTPUT);
  pinMode(pocPin, INPUT);
  Serial.begin(115200);
  // TCCR1B = (TCCR1B & (B11111000 | B00000001));    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR1B = (TCCR1B & (B11111000 | B00000010));    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
  TCCR1B = (TCCR1B & (0b11111000 | 0b00000100));    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz

//    analogWrite(motorPinGo, 255/2);
}

bool motorDirection = 0;
int motorSpeed = 0;
int motorAdd = 1;
bool currentMotor = 0;
int pocValue;

void loop() { 
  // put your main code here, to run repeatedly:
  pocValue = analogRead(pocPin);
  pocValue = map(pocValue, 0, 1023, 0, 255);
  analogWrite(motorPinGo, pocValue);
  Serial.println(pocValue);
//  motorSpeed += motorAdd;
//  if (motorSpeed == 256) {
//    motorAdd = -1;
//  } else if (motorSpeed == 0) {
//    motorAdd = 1;
//    currentMotor = !currentMotor;
//    digitalWrite(PWMCHOOSE, currentMotor);
//  }
  delay(50);
}
