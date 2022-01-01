#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "solarPacket.h"
#include "timedelay.h"
#include "simpleAverage.h"
#include <SoftwareSerial.h>


#define photoResistorPin A0
#define enablePin A2
#define manualTriggerPin A7
// Motor control relay pin
#define relayPin A1
// LED pins
#define willStartPin 6
#define wontStartPin 5
#define sensorOkPin 4
#define sensorFailPin 3

#define HUMIDITY_THRESHOLD 600
#define nightThreshold 200
// how many minutes should we water the plants
#define WATERING_TIME 1000L * 60 * 30
// Delay after watering has stopped
#define DELAY_AFTER_STOP 1000L * 60 * 60 * 4

bool sensorOk = false;
bool willStart = false;
bool willStartPrev = false;
bool nightDetected = false;
bool nightDetectedPrev = false;
bool startedByTimer = false;
int photoResistorReading;
byte watererEnabled = 0; // 0 off, 1, on
unsigned long watererStartTime = 0;
byte systemTriggerReading;
byte systemTriggerButtonState = 0;
// time to delay watering after it is successfuly ended
unsigned long watererDelayTime = 0;

bool systemEnabledPrev = false;
bool systemEnabled = false;
bool systemTriggered = false;

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio (9, 10);

//SoftwareSerial mySerial(7,8); // RX, TX

//
// Topology
//

simpleAverage humidity(5);
simpleAverage daylight(5);
timedelay daylightDelay(1000L * 60 * 5);
timedelay statsDelay(1000L * 5);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xc897f8d075LL, 0x5feddd192fLL };

void print(const unsigned int number) {
  Serial.print(number);
//  mySerial.print(number);
}

void print(const int number) {
	Serial.print(number);
//	mySerial.print(number);
}

void print(const __FlashStringHelper *string)
{
//  mySerial.print(string);
  Serial.print(string);
}

void print(const char string[]) {
	Serial.print(string);
//	mySerial.print(string);
}

void print(const float number) {
  Serial.print(number);
//  mySerial.print(number);
}

void printFloat(const float number) {
  Serial.print(number);
//  mySerial.print(number);
}

void setup(void) {
	Serial.begin(115200);
  // Disable eeprom
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);

	// Setup and configure rf radio
	radio.begin();
	// optionally, increase the delay between retries & # of retries
	radio.setRetries(15,15);
	// optionally, reduce the payload size.  seems to
	// improve reliability
	radio.setPayloadSize(8);
	radio.openWritingPipe(pipes[1]);
	radio.openReadingPipe(1,pipes[0]);
	// Start listening
	radio.startListening();
	// Dump the configuration of the rf unit for debugging
	radio.printDetails();

	pinMode(photoResistorPin, INPUT);
	pinMode(relayPin, OUTPUT);
	pinMode(willStartPin, OUTPUT);
	pinMode(wontStartPin, OUTPUT);
	pinMode(sensorOkPin, OUTPUT);
	pinMode(sensorFailPin,OUTPUT);

	pinMode(enablePin, INPUT_PULLUP);
	pinMode(manualTriggerPin, INPUT_PULLUP);

	// Datalogger serial port
//	mySerial.begin(19200);

  // Indicate on LEDs that we are starting up
	digitalWrite(sensorFailPin, HIGH);
	digitalWrite(wontStartPin, HIGH);
	delay(200);
	digitalWrite(sensorFailPin, LOW);
	digitalWrite(wontStartPin, LOW);
	digitalWrite(sensorOkPin, HIGH);
	digitalWrite(willStartPin, HIGH);
	delay(200);
	digitalWrite(sensorOkPin, LOW);
	digitalWrite(willStartPin, LOW);
	print(F("Water pump controller started.\n"));
}

unsigned long lastReading;
unsigned int counter;
solarPacket packet;
unsigned int elapsedTime;

void loop(void) {

	/*
	 * Read sensors
	 */
	daylightDelay.cycle();
	statsDelay.cycle();

	systemEnabled = !digitalRead(enablePin);
  if (systemEnabled != systemEnabledPrev) {
    systemEnabledPrev = systemEnabled;
    if (systemEnabled) {
      print(F("System Enabled\n"));
    } else {
      print(F("System Disabled\n"));
    }
  }
  systemTriggerReading = !digitalRead(manualTriggerPin);
	if (systemTriggerReading && systemTriggerButtonState == 0) {
    systemTriggerButtonState = 1;
    systemTriggered = true;
    print(F("Enabling manual triggerred\n"));
	} else if (!systemTriggerReading && systemTriggerButtonState == 1) {
    systemTriggerButtonState = 2;
	} else if (systemTriggerReading && systemTriggerButtonState == 2) {
    systemTriggerButtonState = 3;
    print(F("Disabling manual triggerred\n"));
    systemTriggered = false;
	} else if (!systemTriggerReading && systemTriggerButtonState == 3) {
    systemTriggerButtonState = 0;
 }

	// Read daylight sensor
	if (daylightDelay.hasPassed()) {
		daylightDelay.reset();
		photoResistorReading = analogRead(photoResistorPin);
		daylight.add(photoResistorReading);
		print(F("Daylight raw: "));
		print(photoResistorReading);
		print(F(" it is "));
		if (photoResistorReading < nightThreshold) {
			print(F("night."));
		} else {
			print(F("day."));
		}
		print(F("\n"));
	}

	// Read input buttons
	// read if auto is enabled
	// read if start is being triggerred

	// read radio if data is available
	if (radio.available()) {
		digitalWrite(sensorOkPin, HIGH);
		// Dump the payloads until we've gotten everything
		bool done = false;
		while (!done)
		{
			// Fetch the payload, and see if this was the last one.
			done = radio.read( &packet, sizeof packet );
			counter++;
			if (counter > 9999)
				counter = 0;

			if (packet.humidity > 0) {
				humidity.add(packet.humidity);
			} else {
				digitalWrite(sensorOkPin, LOW);
				digitalWrite(sensorFailPin, HIGH);
			}

			float results = (float) (((float) packet.voltage) / 100);
			elapsedTime = (millis () - lastReading) / 1000;
			lastReading = millis();

			// Sensor is OK if reading is received in timely manner and if humidity status is showing
			if (elapsedTime < 100 && packet.humidity != 0) {
				sensorOk = true;
			} else {
				sensorOk = false;
			}

			print(counter);
			print(F(". "));
			print(packet.voltageReading);
			print(F(" Voltage: "));
			print(results);
			print(F(", Humidity: "));
			print(packet.humidity);
			results = (float) (((float) packet.temperature) / 100);
			print(F(", Temp: "));
			print(results);
			print(F(", elapsed time: "));
			print(elapsedTime);
			print(F("             \n"));
		}  // end reading the payload
		digitalWrite(sensorOkPin, LOW);
		digitalWrite(sensorFailPin, LOW);
	} // end radio receive
  
	// Predict if we will start to water plants according to current humidity settings
	if ((humidity.count() > 4) && (humidity.getAverage() < HUMIDITY_THRESHOLD)) {
		willStart = true;
    if (willStartPrev != willStart) {
      print(F("willStart enabled\n"));
    }
    willStartPrev = willStart;
	} else {
		willStart = false;
    if (willStartPrev != willStart) {
      print(F("willStart disabled\n"));
    }
    willStartPrev = willStart;
	}

	if ((daylight.count() > 4) && (daylight.getAverage() < nightThreshold)) {
		nightDetected = true;
    if (nightDetectedPrev != nightDetected) {
      print(F("nightDetected enabled\n"));
    }
    nightDetectedPrev = nightDetected;
	} else {
		nightDetected = false;
    if (nightDetectedPrev != nightDetected) {
      print(F("nightDetected disabled\n"));
    }
    nightDetectedPrev = nightDetected;
	}

	/*
	 * Output state to leds, if time has come
	 */
	if (statsDelay.hasPassed()) {
    elapsedTime = (millis() - lastReading) / 1000;
    if (elapsedTime > 100) {
      sensorOk = false;
    }
    if (elapsedTime > 600) {
      humidity.clear();
    }
		statsDelay.reset();
		if (sensorOk) {
			digitalWrite(sensorOkPin, HIGH);
		} else {
			digitalWrite(sensorFailPin, HIGH);
		}

		if (willStart) {
			digitalWrite(willStartPin, HIGH);
		} else {
			digitalWrite(wontStartPin, HIGH);
		}
		delay(10);
		digitalWrite(sensorOkPin, LOW);
		digitalWrite(sensorFailPin, LOW);
		digitalWrite(willStartPin, LOW);
		digitalWrite(wontStartPin, LOW);
	}

	/*
	 * Trigger water pump, if it's time to do so
	 *
	 * We do that when it is requested by user or
	 *  o night threshold has been reached (last 5 photoresistor readings mean that it is dark)
	 *  o soil humidity is below threshold
	 */
	if (!watererEnabled) {
		if (willStart && nightDetected && watererDelayTime < millis()) {
			watererEnabled = 1;
      systemTriggered = 1;
      systemTriggerButtonState = 2;
			watererStartTime = millis();
			if (systemEnabled) {
				digitalWrite(relayPin, HIGH);
				print(F("Starting water pump of schedule\n"));
			} else {
				print(F("Would start water pump of schedule [system disabled]\n"));
			}
		} else if (systemTriggered) {
			watererEnabled = 1;
			watererStartTime = millis();
			digitalWrite(relayPin, HIGH);
			print(F("Starting water pump of manual request\n"));
		}
	} else {
    // Water is running
    // Stop if manual stop is requested
    if (!systemTriggered) {
      watererEnabled = 0;
      digitalWrite(relayPin, LOW);
      print(F("Stopping water pump of manual request\n"));
    } else if ((millis() - watererStartTime) > WATERING_TIME) {
			watererEnabled = 0;
      systemTriggered = 0;
      systemTriggerButtonState = 0;
			digitalWrite(relayPin, LOW);
			print(F("Stopping water pump of timeout\n"));
      watererDelayTime = millis() + DELAY_AFTER_STOP;
		}
	}
}
