#define PLUSMOTORPIN1 3
#define PLUSMOTORPIN2 5
#define PLUSMOTORPIN3 6
#define MINUSMOTORPIN1 9
#define MINUSMOTORPIN2 10
#define MINUSMOTORPIN3 11

void setup() {
  Serial.begin(115255);
  pinMode(PLUSMOTORPIN1, OUTPUT);
  pinMode(PLUSMOTORPIN2, OUTPUT);
  pinMode(PLUSMOTORPIN3, OUTPUT);
  pinMode(MINUSMOTORPIN1, OUTPUT);
  pinMode(MINUSMOTORPIN2, OUTPUT);
  pinMode(MINUSMOTORPIN3, OUTPUT);

//  setPwmFrequency(PLUSMOTORPIN1);
//  setPwmFrequency(PLUSMOTORPIN2);
//  setPwmFrequency(PLUSMOTORPIN3);

  digitalWrite(PLUSMOTORPIN1, LOW);
  digitalWrite(PLUSMOTORPIN2, LOW);
  digitalWrite(PLUSMOTORPIN3, LOW);
  digitalWrite(MINUSMOTORPIN1, LOW);
  digitalWrite(MINUSMOTORPIN2, LOW);
  digitalWrite(MINUSMOTORPIN3, LOW);
}

byte motorSteps1Plus[6] = {255, 255, 0, 0, 0, 0};
byte motorSteps2Plus[6] = {0, 0, 255, 255, 0, 0};
byte motorSteps3Plus[6] = {0, 0, 0, 0, 255, 255};
byte motorSteps3Minus[6] = {0, HIGH, HIGH, 0, 0, 0};
byte motorSteps2Minus[6] = {HIGH, 0, 0, 0, 0, HIGH};
byte motorSteps1Minus[6] = {0, 0, 0, HIGH, HIGH, 0};

byte thisStep = 0;
int stepsDone = 0;
int stepsGap = 5;
byte wait = 20;

void loop() {
  digitalWrite(MINUSMOTORPIN1, motorSteps1Minus[thisStep]);
  digitalWrite(MINUSMOTORPIN2, motorSteps2Minus[thisStep]);
  digitalWrite(MINUSMOTORPIN3, motorSteps3Minus[thisStep]);
  analogWrite(PLUSMOTORPIN1, motorSteps1Plus[thisStep]);
  analogWrite(PLUSMOTORPIN2, motorSteps2Plus[thisStep]);
  analogWrite(PLUSMOTORPIN3, motorSteps3Plus[thisStep]);
//  delay(10);
//  analogWrite(PLUSMOTORPIN1, 0);
//  analogWrite(PLUSMOTORPIN2, 0);
//  analogWrite(PLUSMOTORPIN3, 0);

  if (++thisStep >= 6) {
    thisStep = 0;
    stepsDone++;
  }
  if (wait > 2 && stepsDone > stepsGap) {
    stepsGap += 20;
    wait--;
    Serial.println(wait);
  }
  delay(wait);
}

void setPwmFrequency(int pin) {
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | 0x01;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | 0x01;
    }
  }
  else if(pin == 3 || pin == 11) {
    TCCR2B = TCCR2B & 0b11111000 | 0x01;
  }
}
