/**
 * Electronic Load
 * http://www.kerrywong.com
 * 10/2013
 * http://www.kerrywong.com/2013/10/24/building-a-constant-currentconstant-power-electronic-load/
 */
#include <SPI.h>
#include <LiquidCrystal.h>
#include <Encoder.h>
#include <SPI.h>

//LCD pins
const int pinRS = 2;
const int pinEn = 3;
const int pinD4 = 4;
const int pinD5 = 7;
const int pinD6 = 8;
const int pinD7 = 9;

//analog pin 5
const int pinLoadVoltage = 5;

//MCP4921 SPI
const int PIN_CS = 10;

const int BTN_RESET = 14;
const int BTN_RANGE_X2 = 15;
const int BTN_MODE = 16;
const int BTN_ENC = 17;

const int IDX_BTN_RESET = 0;
const int IDX_BTN_RANGE_X2 = 1;
const int IDX_BTN_MODE = 2;
const int IDX_BTN_ENC = 3;
const int LOOP_MAX_COUNT = 2000;

int DACGain = 1; //default gain 1xVref. 

int loadMode = 0; // 0: Constant Current, 1: Constant Power
const float EXT_REF_VOLTAGE = 0.333;

LiquidCrystal lcd(pinRS, pinEn, pinD4, pinD5, pinD6, pinD7);
Encoder currentAdjEnc(5, 6);

int buttons[]={BTN_RESET, BTN_RANGE_X2, BTN_MODE, BTN_ENC};
int buttonReadings[3];
int lastButtonStates[]={HIGH, HIGH, HIGH, HIGH};
int currentButtonStates[]={HIGH, HIGH, HIGH, HIGH};
long lastDebounceTime[3];

long oldEncPosition = -999;
long curEncPosition = 0;
int DACSetStep = 1;
int encoderValue = 0;
int loopCounter = 0;
int DACSetValue = 0;

long ADSum = 0;

float vLoad = 0.0;
float setPower = 0.0;
float setCurrent = 0.0;

void setup()
{
    for (int i = 0 ; i < 4 ; i++) {
        pinMode(buttons[i], INPUT);
        digitalWrite(buttons[i], HIGH);
    }
    
    pinMode(PIN_CS, OUTPUT);
    SPI.begin();  
    SPI.setClockDivider(SPI_CLOCK_DIV2);

    lcd.begin(16, 2);
}

void getCurrentEncPosition()
{
    curEncPosition = currentAdjEnc.read() / 4;
    
    if (curEncPosition != oldEncPosition) {
        if (curEncPosition > oldEncPosition) {
            encoderValue += DACSetStep;
            
            if (encoderValue > 4095) encoderValue = 4095;
        } else {
            encoderValue -= DACSetStep;
            
            if (encoderValue < 0) encoderValue = 0;
        }
        
        oldEncPosition = curEncPosition;
        
        if (loadMode == 0) //Constant Current mode
        {
            setDACOutput(encoderValue);
        } else { //Constant Power mode
            setDACOutput(DACSetValue);
        }
    }
}

//Buffered DAC output
void setDACOutput(unsigned int val) 
{
    byte lByte = val & 0xff;
    //                      SHDN     GA            BUF 
    byte hByte = val >> 8 | 0x10 | DACGain << 5 | 1 << 6;
    
    PORTB &= 0xfb;
    SPI.transfer(hByte);
    SPI.transfer(lByte);
    PORTB |= 0x4;    
}

void displayStatus()
{   
    lcd.clear();     

    //average load voltage
    vLoad = ADSum * 1.0 / (float) LOOP_MAX_COUNT / 1024.0 * 5.0  * (0.974 + 21.91) / 0.974;

    if (loadMode == 0) //Constant Current
    {
        float vSense = 1.0 * encoderValue / 4096.0 * EXT_REF_VOLTAGE;
        float i = 3 * 10 * vSense; // 3 sets of MOSFET in parallel, 0.1 ohm
        if  (DACGain == 0) i *=2; // x2
    
        lcd.print("CI,I Set=");
    
        if (i < 1.0) {
            lcd.print(i * 1000,0);
            lcd.print(" mA");
        } else {
            lcd.print(i ,2);
            lcd.print(" A");
        }
    } else { //Constant Power
        setPower = (float) encoderValue / 20.0; //approximately 0-200W

        lcd.print("CP, P Set=");        
        if (vLoad > 0.5) {//minimum 0.5V
            setCurrent = setPower / (float) vLoad;
            
            //desired sense voltage. Since we have 3 
            //sets of MOSFETS, the results are devided
            //by 3 and multipled by the value of the sense
            //resistor
            float vSense = setCurrent / 3.0 * 0.1;
          
            DACSetValue = (int) (vSense/EXT_REF_VOLTAGE * 4096.0 + 0.5);

            if (setCurrent < 10.0) {
                DACGain = 1;                
            } else {
                DACGain = 0;
                DACSetValue = (int) ((float) DACSetValue / 2.0 + 0.5);
            }                        
            
            if (setPower >= 100.0) {
                lcd.print(setPower, 1);
            }
            else {
               lcd.print(setPower, 2);     
            }
        } else {
            setPower = 0;
            lcd.print(setPower, 2);
        }
        
        setDACOutput(DACSetValue);
        lcd.print("W");
    } 
    
    lcd.setCursor(0, 1);
    lcd.print("LOAD V=");
    lcd.print(vLoad, 2);
    lcd.print(" V");        
}

void loop()
{
    int idx = 0;
    
    for (int i = 0 ; i < 4; i++) {
        buttonReadings[i] = digitalRead(buttons[i]);
        
        if (buttonReadings[i] != lastButtonStates[i]) lastDebounceTime[i] = millis();
        
        if (millis() - lastDebounceTime[i] > 50) { //debouncing the buttons
            if (currentButtonStates[i] != buttonReadings[i]) {
                currentButtonStates[i] = buttonReadings[i];                                
                
                //actions
                if (currentButtonStates[i] == LOW) {
                    switch (i) {
                        case IDX_BTN_RESET:
                        //reset output current to 0
                           encoderValue = 0;
                           DACSetValue = 0;
                           break; 
                        case IDX_BTN_RANGE_X2:
                        //switch between 100W/200W maximum power mode
                            DACGain = DACGain == 1? 0 : 1;
                            
                            if (loadMode == 0) {
                                setDACOutput(encoderValue);
                            }
                            break;
                        case IDX_BTN_MODE:
                        //switch between constant current and constant power
                            loadMode = loadMode == 1? 0 : 1;
                            break;
                        case IDX_BTN_ENC:
                        //cycle through different encoder steps: 1/10/100
                            DACSetStep *= 10;
                            
                            if (DACSetStep > 100) DACSetStep = 1;
                            break;
                    }
                }
            }
        }
        
        lastButtonStates[i] = buttonReadings[i];
    }        

    getCurrentEncPosition();    
    loopCounter++;
    
    // used to smooth out the analogRead results
    ADSum += analogRead(pinLoadVoltage); 
    
    if (loopCounter == LOOP_MAX_COUNT) {
        displayStatus();
        loopCounter = 0;
        ADSum = 0;
    }
}

