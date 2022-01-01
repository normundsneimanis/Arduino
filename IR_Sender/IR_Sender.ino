#include "timeDelay.h"
#define outputPin 11
#define led 13

#define IR_START 4490
#define IR_ONE 1680
#define IR_ZERO 560
#define FREQ 19000

#define PACKET_SIZE 32

#define SYSCLOCK 16000000
#define IR_SEND_PWM_START     (TCCR2A |= _BV(COM2B1))
#define IR_SEND_MARK_TIME(time)  My_delay_uSecs(time)
#define IR_SEND_PWM_STOP    (TCCR2A &= ~(_BV(COM2B1)))
#define IR_SEND_CONFIG_KHZ(val) ({ \
    const uint8_t pwmval = SYSCLOCK / 2000 / (val); \
    TCCR2A = _BV(WGM20);  TCCR2B = _BV(WGM22) | _BV(CS20); \
    OCR2A = pwmval; OCR2B = pwmval / 3; })

int packet[PACKET_SIZE] = {IR_START, 
  IR_ONE, IR_ONE, IR_ONE, IR_ONE, IR_ZERO, IR_ZERO, IR_ZERO, IR_ZERO, 
  IR_ONE, IR_ONE, IR_ONE, IR_ONE, IR_ZERO, IR_ZERO, IR_ZERO, IR_ZERO,
  IR_ONE, IR_ONE, IR_ONE, IR_ONE, IR_ZERO, IR_ZERO, IR_ZERO, IR_ZERO,
  IR_ONE, IR_ONE, IR_ONE, IR_ONE, IR_ZERO, IR_ZERO, IR_ZERO,};

timedelay sendDelay(2000);

void  My_delay_uSecs(unsigned int T) {
  if(T){if(T>16000) {delayMicroseconds(T % 1000); delay(T/1000); } else delayMicroseconds(T);};
}

void sendPacket() {
  for (int i = 0; i < PACKET_SIZE; i++) {
    if (i & 1) {
      IR_SEND_PWM_START;
      //delayMicroseconds(packet[i]);
      IR_SEND_MARK_TIME(packet[i]);
    } else {
      IR_SEND_PWM_STOP;
      IR_SEND_MARK_TIME(packet[i]);
    }
    // Tone tryout
//    tone(outputPin, FREQ, packet[i]);
//    delayMicroseconds(200);
    // digitalWrite tryout
//    digitalWrite(outputPin, HIGH);
//    delayMicroseconds(packet[i]);
//    digitalWrite(outputPin, LOW);
//    delayMicroseconds(200);
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(outputPin, OUTPUT);
  pinMode(led, OUTPUT);

//   // set up Timer 2
// TCCR2A = _BV (COM2A0) | _BV(WGM21);  // CTC, toggle OC2A on Compare Match
// TCCR2B = _BV (CS20);   // No prescaler
// OCR2A =  209;          // compare A register value (210 * clock speed)
//                        //  = 13.125 nS , so frequency is 1 / (2 * 13.125) = 38095
IR_SEND_CONFIG_KHZ(20000);

}

void loop() {
  // put your main code here, to run repeatedly:
  sendDelay.cycle();
  if (sendDelay.hasPassed()) {
    digitalWrite(led, HIGH);
    delay(10);
    digitalWrite(led, LOW);
    sendDelay.reset();
    Serial.println("Sending packet");
    sendPacket();
  }
}

