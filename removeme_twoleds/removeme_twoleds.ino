#define PIN1 9
#define PIN2 10

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN1, OUTPUT);
  pinMode(PIN2, OUTPUT);
  Serial.begin(115200);
  randomSeed(analogRead(0));
}

int led1strength = 0;
int led1add = 1;
int led2strength = 255;
int led2add = -1;


void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(PIN1, led1strength);
  led1strength += led1add;
  if (led1strength == 255) {
    delay(random(2000));
    led1add = -1;
  } else if (led1strength == 0) {
    led1add = 1;
  }
  analogWrite(PIN2, led2strength);
  led2strength += led2add;
  if (led2strength == 255) {
    delay(random(2000));
    led2add = -1;
  } else if (led2strength == 0) {
    led2add = 1;
  }
}
