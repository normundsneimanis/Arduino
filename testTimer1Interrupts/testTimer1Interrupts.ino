#include <TimerOne.h>

const int led = LED_BUILTIN;  // the pin with a LED
const int button = 16;
unsigned long period = 100000; // period used in next
unsigned long periodPlus = 100000;

void setup(void)
{
  pinMode(led, OUTPUT);
  pinMode(button, INPUT);
  Timer1.initialize(100000);
  Timer1.attachInterrupt(disableLed); // blinkLED to run every 0.15 seconds
  Serial.begin(115200);
}

unsigned long startMillis2;
unsigned long stopMillis2;
unsigned long startMillis;
unsigned long stopMillis;
unsigned long stopCountPrev = 0;
unsigned long stopCount = 0;
unsigned long millisDiff;
int startMotor = 0;

// The interrupt will blink the LED, and keep
// track of how many times it has blinked.
volatile unsigned long blinkCount = 0; // use volatile for shared variables

void disableLed(void)
{
  if (startMotor == 1) {
    digitalWrite(led, HIGH);
    startMillis =  stopMillis2 = millis();
    startMotor = 2;
    Timer1.setPeriod(period);
  } else  if (startMotor == 2) {
    digitalWrite(led, LOW);
    stopMillis = millis();
    startMotor = 0;
    stopCount++;
    Timer1.setPeriod(periodPlus / 10);
  }
}

void loop(void)
{
  if (!startMotor && digitalRead(button)) {
    startMotor = 1;
    startMillis2 = millis();
  } else {
    if (stopCountPrev < stopCount) {
      noInterrupts();
      millisDiff = stopMillis - startMillis;
      stopCountPrev = stopCount;
      interrupts();
      Serial.print("Stop count: ");
      Serial.print(stopCountPrev);
      Serial.print(". Milliseconds passed since pressing: ");
      Serial.print(millisDiff);
      Serial.print(" Diff after starting: ");
      Serial.println(stopMillis2 - startMillis2);
      if (period < periodPlus*10) {
        period += periodPlus;
      } else {
        period = periodPlus;
      }
    }
  }
  delay(50);
}
