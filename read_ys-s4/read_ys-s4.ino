#define SERIALBUFFERSIZE 512
static uint8_t serialBuffer[SERIALBUFFERSIZE]; // this hold the imcoming string from serial O string
static uint8_t readIndex;
static uint8_t receiverIndex = 0;
static uint8_t packetStart = 0;
static int serialPackets = 0;

union u_float_ {
 byte  b[4];
 float floatvalue;
} u_float;

union u_int16_ {
 byte  b[2];
 int16_t intvalue;
} u_int16;

union u_uint16_ {
 byte  b[2];
 uint16_t uintvalue;
} u_uint16;

uint32_t read32() {
  uint32_t t = read16();
  t |= (uint32_t)read16()<<16;
  return t;
}

#define abs_(x) ((x) > 0?(x):-(x))

uint16_t read16() {
  uint16_t t = read8();
  t |= (uint16_t)read8()<<8;
  return t;
}

uint8_t read8()  {
  return serialBuffer[readIndex++];
}

void read_serial(){
    //grabing data
    
    while(Serial.available() > 0) {
     
        uint8_t c = Serial.read();
        serialBuffer[receiverIndex] = c;
       
        //serialSymb = c;
  
        // Find '$STP' symb
        if (c == 'P' && receiverIndex > 4) {
       
            uint8_t c0 = serialBuffer[receiverIndex-3], c1 = serialBuffer[receiverIndex-2], c2 = serialBuffer[receiverIndex-1], c3 = serialBuffer[receiverIndex];
            if (c0 == '$' && c1 == 'S' && c2 == 'T' && c3 == 'P') {
           
                if (packetStart == 0) {
                  // Start gathering frame
                  packetStart = 1;
                  receiverIndex = 4; // 0..3 in frame == $STP
                  continue;
                } else
                if (packetStart == 1) {
                  // End frame gathering 
                  //serialSymb = receiverIndex;
                  serialPackets++;             
                  //serialReady = 1;
                  packetStart = 0;
                 
                  // Parse frame
                  // 1) Flight mode
                  base_mode = serialBuffer[75];                 
                  // 2) Voltage
                  u_uint16.b[0] = serialBuffer[70];
                  u_uint16.b[1] = serialBuffer[71];
          //uint16_t battVoltage = serialBuffer[71]*256 + serialBuffer[70];
          osd_vbat_A = 10.0f*(float)u_uint16.uintvalue/2048;
                  // 3) GPS
          osd_satellites_visible = serialBuffer[24];
                  // Latitude
                  u_float.b[0] = serialBuffer[4];
                  u_float.b[1] = serialBuffer[5];
                  u_float.b[2] = serialBuffer[6];
                  u_float.b[3] = serialBuffer[7];
                  osd_lat = u_float.floatvalue;
                  // Longitude
                  u_float.b[0] = serialBuffer[8];
                  u_float.b[1] = serialBuffer[9];
                  u_float.b[2] = serialBuffer[10];
                  u_float.b[3] = serialBuffer[11];
                  osd_lon = u_float.floatvalue;
                  // 4) Altitude, decimeters
                  u_int16.b[0] = serialBuffer[48];
                  u_int16.b[1] = serialBuffer[49];
                  osd_alt = 0.1*(float)u_int16.intvalue;
                  // 5) Home distance
          // Distance from the take-off point (high) byte UCHAR, (46), in meters;
          // Distance from the take-off point (low) byte UCHAR, (52), in meters
          osd_home_distance = serialBuffer[46]*256 + serialBuffer[52];
                  // 6) Speed
                  // GPS velx 2 bytes ushort, unit cm / s (50 to 51)
                  u_int16.b[0] = serialBuffer[50];
                  u_int16.b[1] = serialBuffer[51];
                  float vx = (float)u_int16.intvalue;
                  u_int16.b[0] = serialBuffer[94];
                  u_int16.b[1] = serialBuffer[95];
                  float vy = (float)u_int16.intvalue;
                  osd_gpsspeed = 0.036*sqrt(sq(vx) + sq(vy));
                  // 7) Low battery warning
                  // Alarm flag 1 byte UCHAR (78), 1 - vibrate
                }
            }
        }
       
        receiverIndex++;
        if (receiverIndex >= SERIALBUFFERSIZE-1) {
            receiverIndex = 0;
            packetStart = 0;
        }      
    }
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  read_serial();
}
