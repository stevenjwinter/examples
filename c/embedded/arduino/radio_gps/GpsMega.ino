//---------------- RADIO STATION SCANNER CODE---------
// Current IDE: Windows 1.8.7
// 2018 Dec 13
// Uses Si7903 FM Tuner
// GPS
// SD card reader/writer
//
// GPS is serial on Mega Serial1
// SD Card is SPI
// Tuner is I2C
//

// NEOPIXEL 
#include <Adafruit_NeoPixel.h>
#define PIN            4
#define NPIX	       1

#define RED   0x0F0000
#define GREEN 0x000F00
#define BLUE  0x00000F
#define GOLD  0x0F0F00
#define LBLUE 0x1010CC
#define WHITE 0x909090
#define BLACK 0x050505

//
// Radio Includes-------------
// The radio files are local to this project, edit them for specific features.
#include "Si4703_Breakout.h"
#include <Wire.h>

// SD CARD----------------------
#include <SPI.h>
#include <SD.h>
#include <avr/sleep.h>

const int RESET_PIN = 3;
const int SDIO = 20;  // SDA MEGA 20
const int SCLK = 21;  // SCL MEGA 21

// BAUD RATE FOR SERIAL OUTPUT
#define TERMINAL_BAUD 9600

// RADIO DATA ------------------
Si4703_Breakout radio(RESET_PIN, SDIO, SCLK);
char rdsBuffer[1024]; // was 8 bytes

// GPS -------------------------
#include <Adafruit_GPS.h>

// Connect the GPS Power pin to 5V
// Connect the GPS Ground pin to ground
// If using hardware serial (e.g. Arduino Mega):
//   Connect the GPS TX (transmit) pin to Arduino RX1, RX2 or RX3
//   Connect the GPS RX (receive) pin to matching TX1, TX2 or TX3
// Mega uses hardware serial1 (#one)
Adafruit_GPS GPS(&Serial1);

// set up variables using the SD utility library functions:
#define chipSelect 53

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO  true

// this keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy

int lastMinute = -1;  // when to write

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NPIX, PIN, NEO_GRB + NEO_KHZ800);

// SJW: I did not write this horrible thing, cut and paste hackathon eh!
void printDouble( double val, unsigned int precision){
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

   Serial.print (int(val));  //prints the int part
   Serial.print("."); // print the decimal point
   unsigned int frac;
   if(val >= 0)
       frac = (val - int(val)) * precision;
   else
       frac = (int(val)- val ) * precision;
   Serial.print(frac,DEC) ;
} 

//
// Start radio device.  How to check if it really started?
//
void setupRadio()
{

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Turning radio on...");

  radio.powerOn();

  Serial.println("Radio successfully powered on!");

  radio.setVolume(3);
}



void scanLogFreqs()
{
  // Freqs for US  878 to 1080
  int curFreq = 878;
  
  radio.setChannel(curFreq);

  Serial.println("Starting scan...");
  int newFreq = radio.seekUp();
  while (newFreq > curFreq)
    {
      Serial.print("Seeked to ");
      Serial.print(newFreq);
      Serial.print("  ");
      printDouble( newFreq/10.0, 10);
      Serial.print(" RDS data: ");
      radio.readRDS(rdsBuffer, 15000);
      Serial.println(rdsBuffer);      
      delay(100);  // be friendly, delay
      newFreq = radio.seekUp();
      delay(100);
    }
  Serial.println("Done autoscan");
}
 

File getNextLogFile(){

  // Buffer for log file name, DOS names 8.3, technically 12 + 1 (terminating null)
  char filename[15];
  boolean gotName = false;

  int MAX_LOG = 10000;
  for (uint8_t i = 0; i < MAX_LOG; i++) 
  {
    int charPut = sprintf(filename, "LOG_%04d.csv", i);  // 8.3 name constraint

    /*
    Serial.print("checking ");
    Serial.println(filename);
    */
    if (! SD.exists(filename)) {
      gotName = true;
      break;  // Got a name, fall through
    }
  }

  if (gotName == false){
    Serial.print("Error getting log file name. Failing to write log"); 
    return;  // ERROR
  }

  File logfile;
  logfile = SD.open(filename, FILE_WRITE);
  if( ! logfile ) {
    Serial.print("Couldnt create "); 
    Serial.println(filename);
    return;  // ERROR
    // error(3);
  }

  Serial.print("-----------Opened file: "); 
  Serial.println(filename);

  return logfile;
}


void writeLogHeader(File logfile)
{
  // TSV header
  logfile.print  ("FREQ");     sendTab(logfile);
  logfile.print  ("STRNG");    sendTab(logfile);
  logfile.print  ("DATETIME"); sendTab(logfile);
  logfile.print  ("LAT");      sendTab(logfile);
  logfile.print  ("LON");      sendTab(logfile);
  logfile.print  ("QTY");      sendTab(logfile);
  logfile.print  ("NSATS");    // No tab at end
  logfile.println();

}

void writeGpsData(File logfile, int freq, int sigStren)
{
  char charBuf[128];

  Serial.println("Writing data to file");
  // Freq
  // Screen
  Serial.print("Tuner frequency: ");
  Serial.println(freq/10.0);
  // File
  logfile.print(freq/10.0);
  sendTab(logfile);

  // Radio Signal Strenth
  logfile.print(sigStren);
  sendTab(logfile);

  // Date time  format YYYY-MM-DD HH:MM:SS for Python ingestion
  sprintf(charBuf, "%04d-%02d-%02d %02d:%02d:%02d", 
	  GPS.year, GPS.month, GPS.day,
	  GPS.hour, GPS.minute, GPS.seconds);  
  logfile.print(charBuf);
  sendTab(logfile);
  Serial.print("Datetime: ");
  Serial.println(charBuf);

  // Lat
  int latMultip = (GPS.lat == 'S')? -1 : 1;
  logfile.print(convertDegMinToDecDeg(GPS.latitude) * latMultip);
  sendTab(logfile);
  // Lon
  int lonMultip = (GPS.lon == 'W')? -1 : 1;
  logfile.print(convertDegMinToDecDeg(GPS.longitude) * lonMultip );
  sendTab(logfile);
  // Quality
  logfile.print(GPS.fixquality); 
  sendTab(logfile);
  // Satallites
  logfile.println(GPS.satellites);
  // sendTab(logfile);  // No tab at end

  //logfile.println();
  logfile.flush();  // We do a flush here in case someone removes card before we're done.
  // flush close at the end
}

void sendTab(File f){
  f.print('\t');
}

boolean setupSD()
{
  Serial.print("\nInitializing SD card...");

  if (!SD.begin(chipSelect)) 
    {      
      Serial.println("Card init. failed!");
      return false;
    }
  Serial.println("\r\nCard init complete!");
  return true;
}


void setupGPS()
{
  // 9600 NMEA is the default baud rate for MTK - some use 4800
  GPS.begin(9600);
  
  // You can adjust which sentences to have the module emit, below
  
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data for high update rates!
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // uncomment this line to turn on all the available data - for 9600 baud you'll want 1 Hz rate
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_ALLDATA);
  
  // Set the update rate
  // Note you must send both commands below to change both the output rate (how often the position
  // is written to the serial line), and the position fix rate.
  // 1 Hz update rate
  //GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  //GPS.sendCommand(PMTK_API_SET_FIX_CTL_1HZ);
  // 5 Hz update rate- for 9600 baud you'll have to set the output to RMC or RMCGGA only (see above)
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
  GPS.sendCommand(PMTK_API_SET_FIX_CTL_5HZ);
  // 10 Hz update rate - for 9600 baud you'll have to set the output to RMC only (see above)
  // Note the position can only be updated at most 5 times a second so it will lag behind serial output.
  //GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);
  //GPS.sendCommand(PMTK_API_SET_FIX_CTL_5HZ);

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  // the nice thing about this code is you can have a timer0 interrupt go off
  // every 1 millisecond, and read data from the GPS for you. that makes the
  // loop code a heck of a lot easier!
  useInterrupt(false); //true);
  
  delay(1000);
  Serial.println("Done with GPS setup function");

}


void error()
{
  Serial.println("Error starting");
  while(true)
    {
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(1000);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    }
}

void setup()  
{    
  pixels.setBrightness(100);
  pixels.begin();

  setColor(RED);

  Serial.begin(TERMINAL_BAUD);  // Does not need to be super fast for this application.

  Serial.println("Radio scanner program");
  Serial.println("Setting up Radio...");
  setupRadio();

  Serial.println("Setting up GPS...");
  setupGPS();
  Serial.println("Done setting up GPS");

  Serial.println("Setting up SD...");
  if (setupSD() == false)
    {
      error(); // never returns
    }
  Serial.println("Done setting up SD");

  Serial.println("Getting first GPS Data, may need some time...");
  updateGPS();
  Serial.println("Done getting first GPS Data.");
  setColor(GREEN);
}

// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if (GPSECHO)
    if (c) UDR0 = c;  
    // writing direct to UDR0 is much much faster than Serial.print 
    // but only one character can be written at a time. 
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

// converts lat/long from Adafruit
// degree-minute format to decimal-degrees
double convertDegMinToDecDeg (float degMin) {
  double min = 0.0;
  double decDeg = 0.0;
 
  //get the minutes, fmod() requires double
  min = fmod((double)degMin, 100.0);
 
  //rebuild coordinates in decimal degrees
  degMin = (int) ( degMin / 100 );
  decDeg = degMin + ( min / 60 );
 
  return decDeg;
}

void updateGPS()
{
  Serial.println("Getting fresh GPS...");
  while (true)
    {
      char c = GPS.read();
      if(GPS.newNMEAreceived()) 
	{
	  //Serial.print("v");
	  char *stringptr = GPS.lastNMEA();
	  if (!GPS.parse(stringptr))   // this also sets the newNMEAreceived() flag to false
	    {
	      //Serial.print("-^-");
	      continue;
	    }
	  else 
	    {
	      if(GPS.fixquality == 0)
		{
		  Serial.println("\nGPS fix quality == 0, retry");
		  continue;
		}

	      if (GPS.year < 18 || GPS.year > 30)
		{
		  Serial.println("\nGPS corrupt year, retry");
		  continue;
		}

	      //Serial.println("\nGot fresh GPS data!");
	      break;
	    }
	} 
      else 
	{
	  //Serial.print("^");
	  continue; 
	}
    }
}



void loop()
{
  Serial.println("Starting frequency scan");

  // ***** PREP RADIO LOOP ************
  // Freqs for US  878 to 1080
  int curFreq = 878;
  radio.setChannel(curFreq);
  int newFreq = radio.seekUp();

  // ******* OPEN LOG FILE, WRITE HEADER *********
  File logFile = getNextLogFile();
  writeLogHeader(logFile);

  // ****** SCAN *************
  while (newFreq > curFreq)
    {
      setColor(GOLD);
      // Get a new radio station
      Serial.print("Seeked to ");
      Serial.print(newFreq);
      Serial.print("  ");
      printDouble( newFreq/10.0, 10);

      /*
      Serial.print(" RDS data: ");
      radio.readRDS(rdsBuffer, 15000);
      Serial.println(rdsBuffer);      
      delay(100);  // be friendly, delay
      */

      int sigStren = radio.getStrength();
      Serial.print("Sig Stren: ");
      Serial.println(sigStren);

      //Serial.println("Calling GPS.read()");
      setColor(RED);
      updateGPS();
      setColor(GOLD);

      // Set red, writing to SD card
      setColor(RED);
      writeGpsData(logFile, newFreq, sigStren);

      //------------- SEEK UP ----------------------------------
      setColor(BLUE);
      newFreq = radio.seekUp();
      delay(100);  // Settle down radio..
    }
  Serial.println("Done one autoscan, paused");
  logFile.flush();
  logFile.close();
  Serial.println("Done writing logfile, closed file, pause -----------"); 
  setColor(GREEN);
  delay(5000);
}


void setColor(uint32_t c)
{
  pixels.setPixelColor(0, c);
  pixels.show();
}
