static int motorPin1 = 9;
static int motorPin2 = 10;
int motorPinCurrent;
int motorPinOff;
int motorValue = 0;

void setup() {
  Serial.begin(115200);
  TCCR1B = TCCR1B & B11111000 | B00000100;    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
  pinMode(motorPin1,OUTPUT);
  pinMode(motorPin2,OUTPUT);
  DDRB |= (1 << DDB6) | (1 << DDB7);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (motorPinCurrent == motorPin2) {
    motorPinCurrent = motorPin1;
    motorPinOff = motorPin2;
  } else {
    motorPinCurrent = motorPin2;
    motorPinOff = motorPin1;
  }

  int addValue = 10;
  while(1) {
    Serial.print("speed: ");
    Serial.print(motorValue);
    Serial.print(" motorPin: ");
    Serial.println(motorPinCurrent);
    analogWrite(motorPinOff, 0);
    analogWrite(motorPinCurrent, motorValue);
    
    motorValue += addValue;
    delay(100);
    if (motorValue >= 255) {
      motorValue = 255;
      analogWrite(motorPinCurrent, motorValue);
      delay(1000);
      addValue = -10;
    }
    if (motorValue <= 0) {
      motorValue = 0;
      analogWrite(motorPinCurrent, motorValue);
      delay(1000);
      break;
    }
  }
}
