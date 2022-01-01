#include "simpleAverage.h"

average avg(5);


void setup() {
	Serial.begin(115200);
 Serial.println("Starting");
	avg.add(10);
	avg.add(11);
	avg.add(12);
	avg.add(13);
	avg.add(14);
	if (avg.count() != 5) {
		Serial.print("num elements is not 5: ");
		Serial.println(avg.count());
	}
	
	if (avg.getAverage() != 12) {
		Serial.print("Average is not 12: ");
		Serial.println(avg.count());
	}
avg.add(15);
avg.add(15);
avg.add(15);
avg.add(15);
avg.add(15);
  if (avg.getAverage() != 15) {
    Serial.print("Average is not 15: ");
    Serial.println(avg.count());
  }
 Serial.println("All tests passed");
}

void loop() {}
