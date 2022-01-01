/*
 Simple SD Audio bare minimum example, plays file EXAMPLE.AFM from root folder of SD card.
 
 This example shows how to use the SimpleSDAudio library for audio playback.
 You need: 
 - An Arduino with ATmega168/ATmega368 or better
 - An SD-Card connected to Arduinos SPI port (many shields will do)
   -> copy EXAMPLE.AFM on freshly formated SD card into root folder
 - A passive loudspeaker and resistor or better: active speakers (then stereo output will be possible)
 
 Audio signal output is at the following pin:
 - Arduino with ATmega168/328   (many non-mega Arduinos): Pin 9
 - Arduino with ATmega1280/2560 (many mega Arduinos)    : Pin 44
 
 Using passive speaker:	
    Audio-Pin --- -[100 Ohm resistor]- ---- Speaker ---- GND
    
 on 5v arduino Using amplifier / active speaker / line-in etc.:
    Audio-Pin --||--------------[10k resistor]----+----[1k resistor]---- GND
              100nF capacitor                   to amp

 on 3.3v arduino Using amplifier / active speaker / line-in etc.:
    Audio-Pin --||--------------[5k6 resistor]----+----[1k resistor]---- GND
              100nF capacitor                   to amp
 
 See SimpleSDAudio.h or our website for more information:
 http://www.hackerspace-ffm.de/wiki/index.php?title=SimpleSDAudio
 
 created  23 Jun 2012 by Lutz Lisseck
 */
#include "SimpleSDAudio.h"

#define NUMFILES 4
const char * const files[] = {
  "dragon1.afm",
  "flamstrk.afm",
  "gargyl1.afm",
  "ghost2.afm",
};

void playFile(const char * file) {
  // Select file to play
  if(!SdPlay.setFile(file)) {
    Serial.println("Error 2");
    while(1); // Error file not found -> stop.
  }   
  
  // Start playback
  SdPlay.play();
  
  // Let the worker work until playback is finished
  while(!SdPlay.isStopped()) {
    SdPlay.worker();
  }
}

void setup()
{ 
  Serial.begin(115200);
  // If your SD card CS-Pin is not at Pin 4, enable and adapt the following line:
  SdPlay.setSDCSPin(10);
  
  // Init SdPlay and set audio mode
  if (!SdPlay.init(SSDA_MODE_FULLRATE | SSDA_MODE_MONO)) {
    Serial.println("Error 1");
    while(1); // Error while initialization of SD card -> stop.
  } 
}


void loop(void) {
  for(int i = 0; i < NUMFILES; i++) {
    playFile(files[i]);
  }
}