#include <sys/queue.h>

// Change 0 to temperature differrence you see when measure new chip
//#define tempCoefficient 0 * 1.4
#define tempCoefficient 10 * 1.4

void setup () {
  Serial.begin(115200);
  Serial.println("Initiating internal temperature sensor measurement");
}

//unsigned int rawTemperature;

void loop () {
  ADMUX = (bit(REFS1) | bit(REFS0) | bit(MUX3));
  bitSet(ADCSRA, ADSC);  // start a conversion
  while (bit_is_set(ADCSRA, ADSC))
    {}
  bitSet(ADCSRA, ADSC);  // start a conversion
  while (bit_is_set(ADCSRA, ADSC))
    {}

//  rawTemperature = ;
  Serial.println((ADCW - 324.31 + tempCoefficient ) / 1.22, 2);
  delay(1000);
}
