/* 
 *  Compile it for 5V 16MHz version, upload
 *  If it blinks normally, it's 5V version
 *  If blink takes 2x time, it is 3.3V version
 */

void setup() {
  pinMode(13, OUTPUT);
}

void loop() {
  digitalWrite(13, HIGH);
  delay(3000);
  digitalWrite(13, LOW);
  delay(3000);
}
