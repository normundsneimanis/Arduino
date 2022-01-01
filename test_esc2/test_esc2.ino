#include <Servo.h>

#define MIN_SPEED 42
#define MAX_SPEED 100
#define STOP_SPEED 35
#define potPort A0
#define startPort 3

#define MAX_SIGNAL 1800
#define MIN_SIGNAL 200
#define NETRUAL_SIGNAL (MAX_SIGNAL - MIN_SIGNAL) / 2 + MIN_SIGNAL 


Servo esc;
Servo esc2;
int throttle;
int inputThrottle = MIN_SPEED + 1;
int stopMotor = 0;
int stopMotorPrev = 0;
//int addStep = 1;
//int increasing = 1;
int currentThrottle = STOP_SPEED;

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting up");
  pinMode(potPort, INPUT);
  pinMode(startPort, INPUT_PULLUP);
  esc.attach(9);
  esc2.attach(10);
  esc.write(0);
  esc2.write(0);
  delay(500);

  Serial.println("Throttle to 90%");
//  for (int i=0; i<70; i++) {
    throttle = map(1800, 0, 2000, 0, 200);
    esc.write(throttle);
    esc2.write(throttle);
//    delay(20);
//  }
  delay(2000);
//  throttle = map(10, 0, 100, 0, 200);
//  esc.write(throttle);
//
//  delay(3000);
//  throttle = map(50, 0, 100, 0, 200);
//  esc.write(throttle);

  Serial.println("Throttle to 35%");
  throttle = map(700, 0, 2000, 0, 200);
  esc.write(throttle);
  esc2.write(throttle);
//  for(int i=70;i>0;i--){
//    throttle = map(i, 0, 100, 0, 200);
//    esc.write(throttle);
//    delay(20);
//  }
//  

  delay(2000);
  Serial.println("Throttle to 10%");
  throttle = map(12, 0, 100, 0, 200);
  esc.write(throttle);
  esc2.write(throttle);
  delay(500);
  Serial.println("Running");
}

 
void loop() {
  // Debugging speed from console
//  if (Serial.available()) {
//    throttle = Serial.parseInt();
//    Serial.println(throttle);
//    //throttle = map(throttle, 0, 100, 0, 179);
//  }
//  esc.write(throttle);
  delay(100);
  stopMotor = digitalRead(startPort);
  if (!stopMotor) {
    inputThrottle = analogRead(potPort);
    inputThrottle = map(inputThrottle, 0, 1023, MIN_SPEED, MAX_SPEED);
    inputThrottle = inputThrottle;
    if (stopMotorPrev != stopMotor) {
      stopMotorPrev = stopMotor;
      currentThrottle = MIN_SPEED;
    }
  } else {
    stopMotorPrev = stopMotor;
    inputThrottle = STOP_SPEED;
    currentThrottle = STOP_SPEED;
  }

  // Increase or decrease current throtle
  if (inputThrottle > currentThrottle) {
    currentThrottle++;
  }
  if (inputThrottle < currentThrottle) {
    currentThrottle--;
  }
  // Debugging without incremental speed increase
  //currentThrottle = inputThrottle;

  Serial.print("E: ");
  Serial.print(!stopMotor);
  Serial.print(" Throttle: ");
  Serial.print(currentThrottle);
  Serial.print(" Req: ");
  Serial.println(inputThrottle);
  // Write current throttle to engines
  esc.write(map(currentThrottle, 0, 100, 0, 200));
  esc2.write(map(currentThrottle, 0, 100, 0, 200));
  if (currentThrottle == MIN_SPEED + 1 && inputThrottle > currentThrottle) {
    delay(800);
  }
}

