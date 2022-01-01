#define clk  13
#define mosi 11
#define cs_lcd   9
#define dc       10
#define rst      7

void setup() {
  // initialize digital pin OUTLED as an output.
  Serial.begin(115200);
  // set as outputs
  pinMode(clk, OUTPUT);
  pinMode(mosi, OUTPUT);
  pinMode(cs_lcd, OUTPUT);
  pinMode(dc, OUTPUT);
  pinMode(rst, OUTPUT);
}

void enablePort(int port) {
  digitalWrite(port, HIGH);
}

void disablePort(int port) {
  digitalWrite(port, LOW);
}

void loop() {
  Serial.println("On");
  enablePort(clk);
  enablePort(mosi);
  enablePort(cs_lcd);
  enablePort(dc);
  enablePort(rst);
  delay(3000);
  Serial.println("Off");
  disablePort(clk);
  disablePort(mosi);
  disablePort(cs_lcd);
  disablePort(dc);
  disablePort(rst);
  delay(3000);
}
