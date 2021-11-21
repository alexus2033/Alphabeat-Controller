/*
 * Alphabeat_Controller.ino
 *
 * Created: 17/11/2021 10:38:07 AM
 * Author: Alexus2033
 */ 

#include <Arduino.h>
#include "MIDIUSB.h"   // Arduino Pro Micro only(!)
#include <Encoder.h>   // http://www.pjrc.com/teensy/td_libs_Encoder.html
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"  // HT16K33 with 16-segment display
#include "timer.h"          // https://github.com/brunocalou/Timer   
#include <Ewma.h>           // https://github.com/jonnieZG/EWMA

Ewma adcFilter[2] = { (0.11),(0.11) }; 
Adafruit_AlphaNum4 disp[2] = { Adafruit_AlphaNum4(), Adafruit_AlphaNum4()};
Encoder rotaryEnc(14, 15);
long encoderPos  = -999;

//switch serial output on/off
#define DEBUG 1

#if DEBUG == 1
  #define debug(x) Serial.print(x)
  #define debugfm(x,y) Serial.print(x,y)
  #define debugln(x) Serial.println(x)
#else
  #define debug(x)
  #define debugln(x)
#endif

// Configuration-Data
#define playLED1 4
#define playLED2 5

int buttonPin[4] = { 8, 9, 10, 16 };
byte buttonValueOld[4] = { false, false, false, false }; 
byte midiMessage[4] = { 48, 49, 55, 56 };

// Variables
int analogPotPin[2] = { A0, A1 }; 
int analogValOld[2] = { 0, 0};
char puffer[10];
byte minutes[2] = { 0, 0};
byte secs[2] = { 0, 0 };
bool eom[2] = { false, false };
bool blinker = false;
Timer blinkTimer;

// MIDI Data-Format
// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void setup() {
  Serial.begin(115200);
  //turn off boards LEDs
  pinMode(LED_BUILTIN_TX,INPUT);
  pinMode(LED_BUILTIN_RX,INPUT);
  pinMode(playLED1, OUTPUT); 
  pinMode(playLED2, OUTPUT); 
  for (byte x=0; x<sizeof(buttonPin); x++) {
    pinMode(buttonPin[x], INPUT_PULLUP);
  }
  
  //init I2C Displays
  disp[0].begin(0x71);
  disp[1].begin(0x70);
  for (byte x=0; x<2; x++) {
    disp[x].clear();
    disp[x].setBrightness(12);
    //say "hello" from northern germany(!)
    displayTime(x,"MOIN");
  }
  blinkTimer.setInterval(600);
  blinkTimer.setCallback(DoBlink);
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void checkButton(byte ID){

  bool buttonValueNew = digitalRead(buttonPin[ID]);
  if (buttonValueNew != buttonValueOld[ID]){
    if (buttonValueNew == LOW){
      debug("Button pressed: ");
      noteOn(0, midiMessage[ID], 64);
      MidiUSB.flush();
    }
    else {
      debug("Button released: ");
      noteOff(0, midiMessage[ID], 64);  
      MidiUSB.flush();
    }
    debugln(ID);
    buttonValueOld[ID] = buttonValueNew;
  }  
}

void loop() {

  checkButton(0);
  checkButton(1);
  checkButton(2);
  checkButton(3);

  readPoti(0,54);
  readPoti(1,55);
  
  //read encoder
  long newPos = rotaryEnc.read()/4;
  byte diff = newPos - encoderPos; 
  if(diff > 0){
      controlChange(0, 46, 128-diff);
      debugln(encoderPos);
      MidiUSB.flush();
  } else if (diff < 0){
      controlChange(0, 46, diff);
      debugln(encoderPos);
      MidiUSB.flush();
  }
  encoderPos = newPos;

  midiEventPacket_t rx;
  rx = MidiUSB.read();
    if (rx.header != 0) {
      if (rx.header == 0x09 && rx.byte1 == 0x90){
        debug("Received: ");
        debugfm(rx.header, HEX);
        debug("-");
        debugfm(rx.byte1, HEX);
        debug("-");
        debugfm(rx.byte2, HEX);
        debug("-");
        debugfm(rx.byte3, HEX);
        debugln("-");
      }
      // LED for Play-Button 1
      if (rx.header == 0x09 && rx.byte1 == 0x90 && rx.byte2 == 0x01){
        if(rx.byte3 == 0x7F){
          // change blink behaviour: *Preferences > Interface > CUE MODE 
          digitalWrite(playLED1, HIGH);  
        } else {
          digitalWrite(playLED1, LOW);  
        }
      }
      // LED for Play-Button 2
      if (rx.header == 0x09 && rx.byte1 == 0x90 && rx.byte2 == 0x02){
        if(rx.byte3 == 0x7F){
          digitalWrite(playLED2, HIGH); 
        } else {
          digitalWrite(playLED2, LOW); 
        }
      }
      // Pulsing Time Display 1 (EOM)
      if (rx.header == 0x09 && rx.byte1 == 0x90 && rx.byte2 == 0x3e){
        if(rx.byte3 == 0x7F){
           eom[0] = true;
           blinkTimer.start();
        } else {
           eom[0] = false;
           disp[0].setBrightness(12);
        }
      }
      // Pulsing Time Display 2 (EOM)
      if (rx.header == 0x09 && rx.byte1 == 0x90 && rx.byte2 == 0x3f){
        if(rx.byte3 == 0x7F){
           eom[1] = true;
           blinkTimer.start();
        } else {
           eom[1] = false;
           disp[1].setBrightness(12);
        }
      }
      // Time Displays
      if (rx.header == 0x09 && (rx.byte1 == 0x94 || rx.byte1 == 0x95)){
         handleDisplay(rx.byte1-0x94,rx.byte2,rx.byte3);
      }
    }
  blinkTimer.update();
}

//do this every 20ms
void readInputs(){
  checkButton(0);
  checkButton(1);
  checkButton(2);

  readPoti(0,54);
  readPoti(1,55);
}

void handleDisplay(byte displayNr, byte byte2, byte byte3){
  if (byte2 == 0x14){
      minutes[displayNr]=byte3;
  }
  if (byte2 == 0x15){
      secs[displayNr]=byte3;
  }
  if (byte2 == 0x16){
      puffer[0]=0;
      formatTime(minutes[displayNr],secs[displayNr],byte3);
      displayTime(displayNr,puffer);
      if(minutes[displayNr]==0 && secs[displayNr]==0 && byte3==0){
        eom[displayNr] = false;
      }
  }
}

void readPoti(byte potNr, int analogpotCC){
  
  int pot = analogRead(analogPotPin[potNr]);
  int analogpotNew = adcFilter[potNr].filter(pot);
    
  if (analogpotNew - analogValOld[potNr] > 5 || analogValOld[potNr] - analogpotNew > 5) {
    analogValOld[potNr] = analogpotNew;
    analogpotNew = (map(analogpotNew, 0, 1023, 0, 127));
    analogpotNew = (constrain(analogpotNew, 0, 127));
    pot = (map(pot, 0, 1023, 0, 127));
    controlChange(0, analogpotCC, analogpotNew); // Set the value of controller 10 on channel 0 to new value
    MidiUSB.flush();
    debug("pot1filter: ");
    debugln(analogpotNew);
    debug("potread: ");
    debugln(pot);
  }
} 

void displayTest(){
  for (byte y=1; y<15; y++) {  
    for (byte dg=0; dg<=3; dg++) {
      disp[0].writeDigitAscii(dg, y);
      disp[1].writeDigitAscii(dg, y);
    }
    disp[0].writeDisplay();
    disp[1].writeDisplay();
    delay(200);
  }  
}

// format time and add it to the buffer
void formatTime(byte mi, byte sec, byte msec) {
  char buf[7]; // 0.00.0
  if(mi<10){
     buf[0] = (mi % 10) + '0';
     buf[1] = '.';
     buf[2] = (sec / 10) + '0';
     buf[3] = (sec % 10) + '0';
     buf[4] = '.';
     buf[5] = (msec % 10) + '0';
     buf[6] = 0; // terminate with 0
  } else {
     buf[0] = (mi / 10) + '0';
     buf[1] = (mi % 10) + '0';
     buf[2] = '.';
     buf[3] = (sec / 10) + '0';
     buf[4] = (sec % 10) + '0';
     buf[5] = 0; // terminate with 0
  }  
  strcat(puffer,buf);
} 

void displayTime(byte displayNr, char* str) { 
  byte pos = 0;
  for (int x = 0; x < strlen(str); x++)  //for each character in str
  {
    if(str[x] == '.') // if there is a decimal point
    {
      disp[displayNr].writeDigitAscii(pos-1, str[x-1], true);  // go back and rewrite the last digit with the dec point
    }
    else
    {
      disp[displayNr].writeDigitAscii(pos, str[x]);  //write the character
      pos++;
    }
  }
  disp[displayNr].writeDisplay();  //write to the display.
}


void DoBlink(){
  if(eom[0]==false && eom[1]==false){
    blinkTimer.stop();
    return;
  }
  for (byte x=0; x<2; x++) {
    if(eom[x] == true) {
      if(blinker == true){ //pulse display (do not turn off display)
         disp[x].setBrightness(1);
      } else {
         disp[x].setBrightness(12);
      }
    }
  }
  blinker = !blinker;
}
