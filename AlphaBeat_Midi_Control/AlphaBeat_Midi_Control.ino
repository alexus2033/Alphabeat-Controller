/*
 * Alphabeat_Controller.ino
 *
 * Created: 17/11/2021 10:38:07 AM
 * Author: Alexus2033
 */ 

//INT6 (on pin 7) is not edge sensitive :-(
#define ENCODER_DO_NOT_USE_INTERRUPTS 1

#include <Arduino.h>
#include "MIDIUSB.h"   // Arduino Pro Micro only(!)
#include <Encoder.h>   // http://www.pjrc.com/teensy/td_libs_Encoder.html
#include "Adafruit_LEDBackpack.h"  // HT16K33 with 16-segment display
#include "timer.h"          // https://github.com/brunocalou/Timer   
#include <Ewma.h>           // https://github.com/jonnieZG/EWMA

Ewma adcFilter[2] = { (0.11),(0.11) }; 
Adafruit_AlphaNum4 disp[2] = { Adafruit_AlphaNum4(), Adafruit_AlphaNum4()};
Encoder rotaryEnc(7, 6);
long encoderPos = 0;

//switch serial output on/off
#define DEBUG 1
#define DEBUGPOTI 0

#if DEBUG == 1
  #define debug(x) Serial.print(x)
  #define debugfm(x,y) Serial.print(x,y)
  #define debugln(x) Serial.println(x)
#else
  #define debug(x)
  #define debugfm(x,y)
  #define debugln(x)
#endif

#if DEBUGPOTI == 1
  #define debugpoti(x) Serial.print(x)
  #define debugpotiln(x) Serial.println(x)
#else
  #define debugpoti(x)
  #define debugpotiln(x)
#endif

// Configuration-Data
#define playLED1 4
#define playLED2 5
// Sensitifity Poti-Change
#define POTISens 4

#define modeSelect 0
#define modeSpeed 1
#define modeHigh 2
#define modeMid 3
#define modeLow 4
#define modeVolume 5
byte dispMode = modeSelect;
bool modeValueOld = false;

// Buttons
int buttonPin[6] = { 8, 9, 14, 15, 10, 16 };
bool buttonValueOld[6] = { false, false, false, false, false }; 
byte midiMessage[6] = { 50, 51, 55, 56, 58 };

// Variables
char puffer[10];
int analogPotPin[2] = { A1, A0 }; 
int analogValOld[2] = { 0, 0};
byte minutes[2] = { 0, 0};
byte secs[2] = { 0, 0 };
bool eom[2] = { false, false };
byte dispCounter[2] = { 0, 0 };
byte deck1Active = true;
bool blinker = false;
Timer blinkTimer;
Timer dispTimer;

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
#if DEBUG == 1 || DEBUGPOTI == 1
  Serial.begin(115200);
#endif
  pinMode(playLED1, OUTPUT); 
  pinMode(playLED2, OUTPUT); 
  for (byte x=0; x<sizeof(buttonPin); x++) {
    pinMode(buttonPin[x], INPUT);
  }
   
  //init I2C Displays
  disp[0].begin(0x71);
  disp[1].begin(0x70);
  displayTest();
  //turn off boards LEDs
  pinMode(LED_BUILTIN_TX,INPUT);
  pinMode(LED_BUILTIN_RX,INPUT);
  blinkTimer.setInterval(600);
  blinkTimer.setCallback(DoBlink);
  dispTimer.setInterval(5000);
  dispTimer.setCallback(dispAction);
  dispTimer.start();
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
    }
    else {
      debug("Button released: ");
      noteOff(0, midiMessage[ID], 64);  
    }
    MidiUSB.flush();
    debugln(ID);
    buttonValueOld[ID] = buttonValueNew;
  }  
}

//switch Mode for rotary Encoder
void checkModeButton(byte ID){

  bool buttonValueNew = digitalRead(buttonPin[ID]);
  if (buttonValueNew != buttonValueOld[ID]){
    if (buttonValueNew == LOW){
      debug("Button pressed: ");
      dispMode++;
      if(dispMode > 5){
          dispMode = modeSelect;
      }
      byte displayNr = 0;
      if(deck1Active){
        displayNr = 1;
      }      
      if(dispMode == modeSpeed){
        updateDisplay(displayNr,"TEMP");
      } else if (dispMode == modeHigh){
        updateDisplay(displayNr,"HIGH");
      } else if (dispMode == modeMid){
        updateDisplay(displayNr,"mID ");
      } else if (dispMode == modeLow){
        updateDisplay(displayNr,"LOW ");
      } else if (dispMode == modeVolume){
        updateDisplay(displayNr,"VOL ");
      } else {
        updateDisplay(displayNr,"SEL ");
      }
    }
    else {
      debug("Button released: ");
    }
    debugln(ID);
    buttonValueOld[ID] = buttonValueNew;
  }  
}

void loop() {

  checkButton(0);     //play deck 1
  checkButton(1);     //play deck 2
  checkButton(2);     //cue deck 1
  checkButton(3);     //cue deck 2
  checkButton(4);     //select
  checkModeButton(5); //mode

  readEncoder();
    
  readPoti(0,31);
  readPoti(1,41);

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
        debugln("");
      }
      // LED for Play-Button 1
      if (rx.header == 0x09 && rx.byte1 == 0x90 && rx.byte2 == 0x01){
        if(rx.byte3 == 0x7F){
          // change blink behaviour: *Preferences > Interface > CUE MODE 
          digitalWrite(playLED1, HIGH);
          dispCounter[0]=0; //reset inactive counter  
        } else {
          digitalWrite(playLED1, LOW);  
        }
      }
      // LED for Play-Button 2
      if (rx.header == 0x09 && rx.byte1 == 0x90 && rx.byte2 == 0x02){
        if(rx.byte3 == 0x7F){
          digitalWrite(playLED2, HIGH);
          dispCounter[1]=0; //reset inactive counter
        } else {
          digitalWrite(playLED2, LOW); 
        }
      }
      // Pulsing Time Display 1 (EOM)
      if (rx.header == 0x09 && rx.byte1 == 0x90 && rx.byte2 == 0x3a){
        if(rx.byte3 == 0x7F){
           eom[0] = true;
           blinkTimer.start();
        } else {
           eom[0] = false;
           disp[0].setBrightness(12);
        }
      }
      // Pulsing Time Display 2 (EOM)
      if (rx.header == 0x09 && rx.byte1 == 0x90 && rx.byte2 == 0x3b){
        if(rx.byte3 == 0x7F){
           eom[1] = true;
           blinkTimer.start();
        } else {
           eom[1] = false;
           disp[1].setBrightness(12);
        }
      }
      // Reset/Disconnect
      if (rx.header == 0x09 && rx.byte1 == 0x90 && rx.byte2 == 0x3f){
         displayTest();
      }
      // Time Displays
      if (rx.header == 0x09 && (rx.byte1 == 0x94 || rx.byte1 == 0x95)){
         handleDisplay(rx.byte1-0x94,rx.byte2,rx.byte3);
      }
    }
  blinkTimer.update();
  dispTimer.update();
}

//time-critical
void readEncoder(){
  long newPos = rotaryEnc.read()/4;
  long diff = newPos - encoderPos; 

  if(diff != 0){
      if(deck1Active){
        controlChange(0, 32+dispMode, 64+diff); 
      } else{
        controlChange(0, 42+dispMode, 64+diff);
      }
      debug(newPos);
      MidiUSB.flush();
  }
  encoderPos = newPos;
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
      updateDisplay(displayNr,puffer);
      dispCounter[displayNr]=0;
      deck1Active = (displayNr==0);
  }
}

void readPoti(byte potNr, int analogpotCC){
  
  int pot = analogRead(analogPotPin[potNr]);
  int analogpotNew = adcFilter[potNr].filter(pot);
    
  if (analogpotNew - analogValOld[potNr] > POTISens || analogValOld[potNr] - analogpotNew > POTISens) {
    analogValOld[potNr] = analogpotNew;
    analogpotNew = (map(analogpotNew, 0, 1023, 0, 127));
    analogpotNew = (constrain(analogpotNew, 0, 127));
    pot = (map(pot, 0, 1023, 0, 127));
    controlChange(0, analogpotCC, analogpotNew); // Set the value of controller 10 on channel 0 to new value
    MidiUSB.flush();
    debugpoti("potFilter: ");
    debugpotiln(analogpotNew);
    debugpoti("potRead: ");
    debugpotiln(pot);
  }
} 

//runs every 5 Secs.
void dispAction(){
  dispCounter[0]++;
  dispCounter[1]++;
  if(dispCounter[0] >= 96){       //after 8 Min check activity
    debugln("Check dispMode");
    updateDisplay(0," .  . ");   
  }
  if(dispCounter[1] >= 96){ 
    updateDisplay(1," .  . ");
  }
  //avoid overflow
  if(dispCounter[0]>250){
    dispCounter[0]==250;
  }
  if(dispCounter[1]>250){
    dispCounter[1]==250;
  }
}

void displayTest(){
  dispCounter[0] = 0;
  dispCounter[1] = 0;
  byte test[15] = {14,13,7,10,9,8,6,11,12,2,1,0,5,4,3};  
  digitalWrite(playLED1, HIGH);
  digitalWrite(playLED2, HIGH);
  for (byte y=0; y<15; y++) {  
    for (byte dg=0; dg<=3; dg++) {
      disp[0].writeDigitAscii(dg, test[y]);
      disp[1].writeDigitAscii(dg, test[y]);
    }
    disp[0].writeDisplay();
    disp[1].writeDisplay();
    delay(200);
  }  
  digitalWrite(playLED1, LOW);
  digitalWrite(playLED2, LOW);
  for (byte x=0; x<2; x++) {
    eom[x] = false;
    disp[x].clear();
    disp[x].setBrightness(12);
    //say "hello" from northern germany(!)
    updateDisplay(x,"MOIN");
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

void updateDisplay(byte displayNr, char* str) { 
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
