#define DATAOUT 11//MOSI
#define DATAIN  12//MISO
#define SPICLOCK  13//sck
#define SLAVESELECT 8 //ss

//opcodes
#define WREN  0x06
#define WRDI  0x04
#define RDSR  0x05
#define WRSR  0x01
#define READ  0x03
#define WRITE 0x02

byte eeprom_output_data;
byte eeprom_input_data=0;
byte clr;
int address=0;
//data buffer
char buffer[128];

void fill_buffer()
{
  for (int I=0;I<15;I++)
  {
    buffer[I]=I+49;
    Serial.print("Buffering ");
    Serial.print(buffer[I], HEX);
    Serial.print(" to ");
    Serial.println(I);
  }
}

char spi_transfer(volatile char data)
{
  SPDR = data;                    // Start the transmission
  while (!(SPSR & (1<<SPIF)))     // Wait the end of the transmission
  {
  };
  return SPDR;                    // return the received byte
}

void setup()
{
  Serial.begin(115200);

  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK, OUTPUT);
  pinMode(SLAVESELECT, OUTPUT);
  digitalWrite(SLAVESELECT, HIGH); //disable device
  // this is to disable other device on SPI bus
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);

  // SPCR = 01010000
  //interrupt disabled,spi enabled,msb 1st,master,clk low when idle,
  //sample on leading edge of clk,system clock/4 rate (fastest)
  SPCR = (1<<SPE)|(1<<MSTR);
  clr=SPSR;
  clr=SPDR;
  delay(10);
  //fill buffer with data
  fill_buffer();
  //fill eeprom w/ buffer
  digitalWrite(SLAVESELECT, LOW);
  spi_transfer(WREN); //write enable
  digitalWrite(SLAVESELECT, HIGH);
//  delay(10);
  digitalWrite(SLAVESELECT, LOW);
  spi_transfer(WRITE); //write instruction
  address=0;
  spi_transfer((char)(address>>8));   //send MSByte address first
  spi_transfer((char)(address));      //send LSByte address
  //write 128 bytes
  for (int I=0; I<15; I++) {
    spi_transfer(buffer[I]); //write data byte
    Serial.print("Writing ");
    Serial.print(buffer[I], HEX);
    Serial.print(" to address ");
    Serial.println(I);
  }
  digitalWrite(SLAVESELECT, HIGH); //release chip
  //wait for eeprom to finish writing
//  delay(1000);
  Serial.println("Write finished");
}

byte read_eeprom(int EEPROM_address)
{
  //READ EEPROM
  int data;
  digitalWrite(SLAVESELECT, LOW);
  spi_transfer(READ); //transmit read opcode
  spi_transfer((char)(EEPROM_address>>8));   //send MSByte address first
  spi_transfer((char)(EEPROM_address));      //send LSByte address
  data = spi_transfer(READ); //get data byte
  digitalWrite(SLAVESELECT, HIGH); //release chip, signal end transfer
  return data;
}

void loop()
{
  eeprom_output_data = read_eeprom(address);
  Serial.print("Reading from adress ");
  Serial.print(address);
  Serial.print(": " );
  Serial.println(eeprom_output_data, HEX);
  address++;
  if (address == 15) {
    address = 0;
    delay(60000);
  }
//  delay(5000); //pause for readability
}
