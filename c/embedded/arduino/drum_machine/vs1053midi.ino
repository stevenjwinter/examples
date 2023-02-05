/*************************************************** 
  This is an example for the Adafruit VS1053 Codec Breakout

  Designed specifically to work with the Adafruit VS1053 Codec Breakout 
  ----> https://www.adafruit.com/products/1381

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

// define the pins used
#define VS1053_RX  2 // This is the pin that connects to the RX pin on VS1053

#define VS1053_RESET 9 // This is the pin that connects to the RESET pin on VS1053
// If you have the Music Maker shield, you don't need to connect the RESET pin!

// If you're using the VS1053 breakout:
// Don't forget to connect the GPIO #0 to GROUND and GPIO #1 pin to 3.3V
// If you're using the Music Maker shield:
// Don't forget to connect the GPIO #1 pin to 3.3V and the RX pin to digital #2

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 31
#define VS1053_BANK_DEFAULT 0x00
#define MIDI_GLOBAL_VOLUME 0x01
#define VS1053_BANK_DRUMS1 0x78
#define VS1053_BANK_DRUMS2 0x7F
#define VS1053_BANK_MELODY 0x79

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 32 for more!
#define VS1053_GM1_OCARINA 80

#define MIDI_NOTE_ON  0x90
#define MIDI_NOTE_OFF 0x80
#define MIDI_CHAN_MSG 0xB0
#define MIDI_CHAN_BANK 0x00
#define MIDI_CHAN_VOLUME 0x07
#define MIDI_CHAN_PROGRAM 0xC0


#define PIN            8

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      7

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


SoftwareSerial VS1053_MIDI(0, 2); // TX only, do not use the 'rx' side
// on a Mega/Leonardo you may have to change the pin to one that 
// software serial support uses OR use a hardware serial port!

#define KICK_BUT  6
#define SNAR_BUT  5
#define HAT_BUT   7
#define PER_BUT   4

#define KICK_LED  11
#define SNAR_LED  12
#define HAT_LED   13
#define PER_LED   10


#define KICK_NOTE  36  // bass drum
#define SNARE_NOTE 40  // Elect snare
#define HAT_NOTE   42
#define PER_NOTE   56


bool kicked = false;
bool snared = false;
bool hatted = false;
bool perred = false;


void setup() {

  Serial.begin(9600);
  Serial.println("starting VS1053 MIDI mode");
  
  pinMode(KICK_BUT, INPUT);     
  pinMode(SNAR_BUT, INPUT);     
  pinMode(HAT_BUT, INPUT);     
  pinMode(PER_BUT, INPUT);     

  pinMode(KICK_LED, OUTPUT);
  pinMode(SNAR_LED, OUTPUT);
  pinMode(HAT_LED,  OUTPUT);
  pinMode(PER_LED,  OUTPUT);

  VS1053_MIDI.begin(31250); // MIDI uses a 'strange baud rate'
  
  pinMode(VS1053_RESET, OUTPUT);
  digitalWrite(VS1053_RESET, LOW);
  delay(10);
  digitalWrite(VS1053_RESET, HIGH);
  delay(10);
  
  midiSetChannelBank(0, VS1053_BANK_DRUMS2);
  midiSetInstrument(0, VS1053_GM1_OCARINA);
  midiSetChannelVolume(0, 127);
  Serial.println("VS1053 Done init");

  pixels.begin(); // This initializes the NeoPixel library.
  
  fillPixels(0xFF0000);

  Serial.println("Neopixels Done init");


  midiSetVolume(126); // Turn it up to 11, which is not really that loud.

}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


long nextTime = 0;
long TIME_BUMP = 100;

byte wheelPos = 0;

void loop() {  

  long now = millis();
  if (now>nextTime)
    {
      wheelPos++;
      if (wheelPos >= 255)
	wheelPos = 0;

      uint32_t c = Wheel(wheelPos);
      fillPixels(c);
      nextTime = now + TIME_BUMP;
   }


  int DRUM_CHANNEL = 0;

  int butState = digitalRead(KICK_BUT);

  if (butState == LOW)
    {
      if (kicked == false ) {     
	kicked = true;
	midiNoteOn(DRUM_CHANNEL, KICK_NOTE, 127);
	digitalWrite(KICK_LED, HIGH);  

      }
    } 
  else 
    {
      if (kicked == true){
      midiNoteOff(DRUM_CHANNEL, KICK_NOTE, 127);
      digitalWrite(KICK_LED, LOW);  
      kicked = false;
      }
    }

  butState = digitalRead(SNAR_BUT);
  if (butState == LOW)
    {
      if (snared == false ) {     
	snared = true;
	midiNoteOn(DRUM_CHANNEL, SNARE_NOTE, 127);
	digitalWrite(SNAR_LED, HIGH);  
      }
    } 
  else 
    {
      if (snared == true){
      midiNoteOff(DRUM_CHANNEL, SNARE_NOTE, 127);
      snared = false;
      digitalWrite(SNAR_LED, LOW);  
      }
    }

  butState = digitalRead(HAT_BUT);
  if (butState == LOW)
    {
      if (hatted == false ) {     
	hatted = true;
	midiNoteOn(DRUM_CHANNEL, HAT_NOTE, 127);
	digitalWrite(HAT_LED, HIGH);  
      }
    } 
  else 
    {
      if (hatted == true){
      midiNoteOff(DRUM_CHANNEL, HAT_NOTE, 127);
      hatted = false;
      digitalWrite(HAT_LED, LOW);  
      }
    }

  butState = digitalRead(PER_BUT);
  if (butState == LOW)
    {
      if (perred == false ) {     
	perred = true;
	midiNoteOn(DRUM_CHANNEL, PER_NOTE, 127);
	digitalWrite(PER_LED, HIGH);  
      }
    } 
  else 
    {
      if (perred == true){
      midiNoteOff(DRUM_CHANNEL, PER_NOTE, 127);
      perred = false;
      digitalWrite(PER_LED, LOW);  
      }
    }
}

void midiSetInstrument(uint8_t chan, uint8_t inst) {
  if (chan > 15) return;
  inst --; // page 32 has instruments starting with 1 not 0 :(
  if (inst > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_PROGRAM | chan);  
  VS1053_MIDI.write(inst);
}

void midiSetVolume(uint8_t vol) {
  if (vol > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_VOLUME);
  VS1053_MIDI.write(vol);
}


void midiSetChannelVolume(uint8_t chan, uint8_t vol) {
  if (chan > 15) return;
  if (vol > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write(MIDI_CHAN_VOLUME);
  VS1053_MIDI.write(vol);
}

void midiSetChannelBank(uint8_t chan, uint8_t bank) {
  if (chan > 15) return;
  if (bank > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write((uint8_t)MIDI_CHAN_BANK);
  VS1053_MIDI.write(bank);
}

void midiNoteOn(uint8_t chan, uint8_t n, uint8_t vel) {

  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;

  //Serial.println("Note On");

  wheelPos = random(1, 200);

  VS1053_MIDI.write(MIDI_NOTE_ON | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}

void midiNoteOff(uint8_t chan, uint8_t n, uint8_t vel) {

  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;
  
  // Serial.println("Note Off");
  VS1053_MIDI.write(MIDI_NOTE_OFF | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}


void fillPixels(uint32_t c)
{
  for(int i=0; i<pixels.numPixels(); i++) 
    {
      pixels.setPixelColor(i, c);
    }
  pixels.show();
}
