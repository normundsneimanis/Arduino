#include <Servo.h>

#define MIN_SPEED 45

Servo esc;
Servo esc2;
int throttle;
int inputThrottle = MIN_SPEED;
int addStep = 1;
int increasing = 1;

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting up");
  esc.attach(9);
  esc2.attach(10);
  esc.write(0);
  esc2.write(0);
  delay(1000);

  Serial.println("Throttle to 90%");
//  for (int i=0; i<70; i++) {
    throttle = map(1800, 0, 2000, 0, 200);
    esc.write(throttle);
    esc2.write(throttle);
//    delay(20);
//  }
  delay(1000);
//  throttle = map(10, 0, 100, 0, 200);
//  esc.write(throttle);
//
//  delay(3000);
//  throttle = map(50, 0, 100, 0, 200);
//  esc.write(throttle);

  Serial.println("Throttle to 700");
  throttle = map(700, 0, 2000, 0, 200);
  esc.write(throttle);
  esc2.write(throttle);
//  for(int i=70;i>0;i--){
//    throttle = map(i, 0, 100, 0, 200);
//    esc.write(throttle);
//    delay(20);
//  }
//  

  delay(1000);
  Serial.println("Throttle to 10%");
  throttle = map(12, 0, 100, 0, 200);
  esc.write(throttle);
  esc2.write(throttle);
  delay(200);
  Serial.println("Running");
}

 
void loop()
{
//  if (Serial.available()) {
//    throttle = Serial.parseInt();
//    Serial.println(throttle);
//  }
//  throttle = map(throttle, 0, 100, 0, 200);
//  esc.write(throttle);
  delay(1000);
  inputThrottle += addStep;
  Serial.print("Throttle to ");
  Serial.println(inputThrottle);
  throttle = map(inputThrottle, 0, 100, 0, 200);
  esc.write(throttle);
  esc2.write(throttle);
  if (inputThrottle == MIN_SPEED + 1) {
    Serial.println("Min +1 reached");
    delay(5000);
  }
  if (increasing && inputThrottle == 30) {
    addStep = 1;
  }
  if (increasing == 0 && inputThrottle < 30) {
    addStep = -1;
  }
  if (inputThrottle == 55) {
    increasing = 0;
    addStep = -1;
  } else if (inputThrottle <= MIN_SPEED) {
    Serial.println("Min reached");
    delay(10000);
    increasing = 1;
    addStep = 1;
  }
}
