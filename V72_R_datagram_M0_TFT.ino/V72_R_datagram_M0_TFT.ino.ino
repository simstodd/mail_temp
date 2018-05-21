// V71_R_datagram_M0_TFT.ino sample code with my changes rf95_reliable_datagram_server.pde
// rf95 demo RX (SERVER) on M0 - lora feather with TFT ILI9341 Weather shield type display
//
//Need to add board support for adafruit in the preferences pane for 3rd party boards
//https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
//Need to add adafruit GFX and ILI9341 support from the add library manager
//add Arduino SAMD Boards support 
//need to add adafruit samd support
//need to add adafruit DS3231 .zip https://github.com/adafruit/RTClib  jeelabs fork
//
//
//
//https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module?view=all
//https://randomnerdtutorials.com/build-an-all-in-one-esp32-weather-station-shield/
//
//http://www.rocketscream.com/blog/2017/08/21/the-sx1276-modules-shootout-hoperfs-rfm95w-vs-nicerfs-lora1276-c1-vs-hpdteks-hpd13/
//https://github.com/PaulStoffregen/RadioHead/blob/master/RH_RF95.h
//http://www.airspayce.com/mikem/arduino/RadioHead/classRHReliableDatagram.html
// adafruit 2.4" tft lcd with touchscreen w/micro sd 
// size 240x320
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging server
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with v64 32u4 rf95_reliable_datagram_client
//https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module?view=all
//http://www.airspayce.com/mikem/arduino/RadioHead/classRHReliableDatagram.html
//https://www.tapatalk.com/groups/talk2/whisper-node-f4/
//https://github.com/simstodd/mail_temp
//https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module?view=all
//battery managament
//working with v64 32u4 client, workign on tft drawing now 
//basic string parsing works, need to add bmps arrange screen, change fonts
//
// https://learn.adafruit.com/adafruit-feather-m0-adalogger/power-management

//
//          maillast time not correct
//          Alert state not resetting, maybe chage to boolean
//          Add graphics for weather
//          
//

//

//

//
#define VBATPIN A7

/*   //almost ready to add this, more important on sensor board
float measuredvbat = analogRead(VBATPIN);
measuredvbat *= 2;    // we divided by 2, so multiply back
measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
measuredvbat /= 1024; // convert to voltage
Serial.print("VBat: " ); Serial.println(measuredvbat);
*/

/*
S: A:0 T:63.82 H:48.88 P:1018.209
Confirmation failed (no ack)
Message From #2, RSSI:-37, S: A:0 T:63.84 H:48.88 P:1018.210
Temp: 63.84
Humid: 48.88
Pressure: 1018.21

Status: 0
56
S: A:0 T:63.84 H:48.88 P:1018.210
Confirmation failed (no ack)
Message From #2, RSSI:-37, S: A:0 T:63.82 H:48.82 P:1018.211
Temp: 63.82
Humid: 48.82
Pressure: 1018.21
Status: 0
50
*/


#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <SD.h>
//#include <Fonts/FreeSerif12pt7b.h> //not found
//need to play with different fonts to customize the display
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "RTClib.h"

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


#define CLIENT_ADDRESS 1 //the 32u4 is the client/peripheral with better power saving
#define SERVER_ADDRESS 2 //the M0 is the server, like the gateway
#define RF95_FREQ 915.0
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
#define TFT_DC 10
#define TFT_CS 9
#define SD_CS 5


//packet counter
int16_t packetnum = 0;  // packet counter, we increment per xmission
//packet counter

  int elapsedTime = 0;
  int lastTime = 0;
  int lastMail = 0;
  int mailLastTime = 0;
  int now_alertTime = 0;
  bool mail_yes = false;
  
//int StatusM = LOW; //keep this state for 12 hours or so
//const int ledPin = LED_BUILTIN; //the number of the led pin
//int  alertState = LOW; //used to set the mailbox alert state
//unsigned long previousMillis = 0; //will store the last time the led was updated
//const long interval = 150000; //interval to displag mailbox message
int lastmail = 00000000;
//tft instance
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
//tft instance

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS,RFM95_INT);
// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);


//DateTime mail_last = rtc.now(); //this needds to be global as well


void setup() 
{
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
    Serial.println("init failed");
    delay(500);
   // driver.setFrequency(RF95_FREQ);  //after init set freq just to be certain.
   //this seems to break things

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

/*  
RH_RF95::ModemConfig modem_config = {
  0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
  0xc4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
  0x0c  // Reg 0x26: LowDataRate=On, Agc=On
};

rf95.setModemRegisters(&modem_config);


//rf95.printRegisters();  //Print all the RFM95 register values
  //rf95.setFrequency(915);  //just to be certain.  
  //rf95.setModemConfig(RH_RF95::Bw31_25Cr48Sf512);  //set for pre-configured long range
  //rf95.setTxPower(20);  //set for 100mw
   // rf95.setTxPower(20, true);  // range from 14-20 for power, 
   
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
//  driver.setTxPower(23, false);
  
 
  // You can optionally require this module to wait until Channel Activity
  // Detection shows no activity on the channel before transmitting by setting
  // the CAD timeout to non-zero:
//  driver.setCADTimeout(10000);
*/
   
delay(500);
Serial.println("V71_R_datagram_M0_TFT");
Serial.print(F("RFM95 Server radio @"));  Serial.print((int)RF95_FREQ);  Serial.println(F(" MHz"));
delay(500);

tft.begin();

//uint8_t x = tft.readcommand8(ILI9341_RDMODE);
//  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
//  x = tft.readcommand8(ILI9341_RDMADCTL);
//  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
//  x = tft.readcommand8(ILI9341_RDPIXFMT);
//  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
//  x = tft.readcommand8(ILI9341_RDIMGFMT);
//  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
//  x = tft.readcommand8(ILI9341_RDSELFDIAG);
//  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
//  Serial.println(testFastLines(ILI9341_CYAN, ILI9341_PINK));
 
 tft.setRotation(1);
 tft.setCursor(0, 0);
 tft.setTextColor(ILI9341_LIGHTGREY);  tft.setTextSize(1);
 tft.fillScreen(ILI9341_BLACK);
   delay(4000);
  tft.print("M0/TFT/LoRA receiver test# v71 ");
  delay(4000);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(50,50);
  tft.print("M0/TFT/LoRA receiver test# v71 ");
  Serial.println("screen test");
  delay(4000);
tft.fillScreen(ILI9341_BLACK);
Serial.println("Testing radio");
Serial.println(driver.printRegisters());
Serial.println("Radio test done, setup done; waiting for first packet.....");
// pinMode(LED_BUILTIN, OUTPUT);     
// pinMode(RFM95_RST, OUTPUT); //doesn't seem to like this radio just sits

} //end setup 

uint8_t data[] = "Hello litle 32u4 from v71 M0 server (0x2) with TFT";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];


void loop()
{
  unsigned long currentMillis = millis();

  DateTime now = rtc.now();
  int nowish = now.unixtime();
  if (manager.available())
  {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from))
    {
      Serial.print("Message from: 0x");
      Serial.print(from, HEX);
      Serial.print("; \" ");
      Serial.print((char*)buf);
      tft.fillScreen(ILI9341_BLACK);
      displayTextW();
      //get local time
     // Serial.println('\"');
    DateTime now = rtc.now();
   // Serial.print("Unix time is now: ");
  // Serial.println(rtc.now());
    elapsedTime = nowish - lastTime;
    Serial.print("last message: ");Serial.print(elapsedTime);Serial.println(" seconds ago");
    
    Serial.print(now.year(), DEC);Serial.print('/');Serial.print(now.month(), DEC);
    Serial.print('/');Serial.print(now.day(), DEC);Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);Serial.print(") ");Serial.print(now.hour(), DEC);
    Serial.print(':');Serial.print(now.minute(), DEC);Serial.print(':');Serial.print(now.second(), DEC);
    //set alertState

    //time1 = 0; //reset counter
     //done setAlert State 
      //get rssi
      Serial.print(", RSSI:");
      Serial.println(driver.lastRssi());
      String myString = (char*)buf;
      //examine
          if (myString.substring(10, 11) > "0") {
                      Serial.print("A = ");
                      Serial.print(myString.substring(10,11));
                      mail_yes = true;
                      Serial.print(mail_yes);
                     // alertState = 1;
                      Serial.print(", Alert/Mailbox motion at (Unix) ");
                      DateTime now_alert = rtc.now();
                      now_alertTime = now_alert.unixtime();
                      Serial.print(now_alertTime);
                      
                      Serial.print(" seconds, or ");
                      elapsedTime = 0;
                      Serial.print(now.hour(), DEC);
                      Serial.print(':');
                      Serial.print(now.minute(), DEC);
                      Serial.print(':');
                      Serial.println(now.second(), DEC);
                         }
      //done examine                   
      // Send a reply back to the originator client
      if (!manager.sendtoWait(data, sizeof(data), from))
        Serial.println("sendtoWait failed");
    }
    lastTime = now.unixtime(); //last time a packet was received
  }
 
}//end main loop

/*
unsigned long testFastLines(uint16_t color1, uint16_t color2) {
  unsigned long start;
  int           x, y, w = tft.width(), h = tft.height();
  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for(y=0; y<h; y+=5) tft.drawFastHLine(0, y, w, color1);
  for(x=0; x<w; x+=5) tft.drawFastVLine(x, 0, h, color2);
  return micros() - start;
}//end of testFastLines
*/



/*
unsigned long testText() {
  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_LIGHTGREY);  tft.setTextSize(1);
  tft.print("M0/TFT/LoRA receiver test# ");
  tft.setTextColor(ILI9341_PINK);
  tft.println( 51);
  tft.println();
  tft.setTextColor(ILI9341_GREEN);  //  tft.setTextSize(2);
  tft.println("Hello, Todd");
  tft.println();
  tft.setTextColor(ILI9341_GREENYELLOW);
  tft.setTextSize(1);
  tft.print("Receiving sensor data");
  tft.setTextColor(ILI9341_YELLOW);
  for(uint8_t dots=0; dots<18; dots++) {
  tft.print(".");
  delay(500);
  }
  tft.println( );
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_CYAN);
  tft.print("Reading Humidity,");
  delay(1000);
  tft.setTextColor(ILI9341_GREEN);
  tft.print(" Temperature,");
  tft.setTextColor(ILI9341_MAGENTA);
  delay(1000);
  tft.println(" Pressure");
  tft.setTextColor(ILI9341_ORANGE);
  tft.println("Sending confirmation... ");
  delay(15000);
  return micros() - start;
}//end of testText

unsigned long testFastLines(uint16_t color1, uint16_t color2) {
  unsigned long start;
  int           x, y, w = tft.width(), h = tft.height();
  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for(y=0; y<h; y+=5) tft.drawFastHLine(0, y, w, color1);
  for(x=0; x<w; x+=5) tft.drawFastVLine(x, 0, h, color2);
  return micros() - start;
}//end of testFastLines
*/


void displayTextW()
{
    String myString = (char*)buf;
    tft.setRotation(1);
   // tft.fillScreen(ILI9341_BLACK);
    //bmpDraw("/big/clear.bmp",70,0);
    
//clear screen
tft.fillScreen(ILI9341_BLACK);
//   

//last message stamp
      tft.setTextColor(ILI9341_LIGHTGREY);  tft.setTextSize(1);
      tft.setCursor(0, 230);
      tft.print("last message at: ");
      DateTime now_alertme = rtc.now();
      tft.print(now_alertme.hour(), DEC);
      tft.print(':');
     tft.print(now_alertme.minute(), DEC);
      tft.print(':');
      tft.println(now_alertme.second(), DEC);
     // tft.println(daysOfTheWeek[now_alertme.dayOfTheWeek()]);
//last message       

    //pressure
    tft.setTextColor(ILI9341_ORANGE);  tft.setTextSize(2);
    tft.setCursor(0, 60);
    tft.println("Pressure:");
    tft.setTextColor(ILI9341_LIGHTGREY);
    tft.setCursor(120, 60);
    //tft.setFont(&FreeSerif12pt7b);
    tft.println(myString.substring(30, 37));
    
  // tft.setCursor(120, 80);
   // tft.println(myString.substring(29, 34));

  //  tft.setCursor(120, 100);
  //  tft.println(myString.substring(33, 37));
    
  //Status: A:0 T:70.68 H:31.40 P:1005.74 #46
  //12345678901234567890123456789012345678901
  //         10        2         3         4 
    //TEMP   
    //tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_GREENYELLOW);  tft.setTextSize(2);
    tft.setCursor(0, 90);
    tft.println("Temp:");
    tft.setTextColor(ILI9341_CYAN);
    tft.setCursor(120, 90);
    //tft.setFont(&FreeSerif12pt7b);
    tft.println(myString.substring(14, 19));

    //HUMID
    tft.setTextColor(ILI9341_ORANGE);  tft.setTextSize(2);
    tft.setCursor(0, 120);
    tft.println("Humid:");
    tft.setTextColor(ILI9341_LIGHTGREY);
    tft.setCursor(120, 120);
    //tft.setFont(&FreeSerif12pt7b);
    tft.println(myString.substring(22, 27));
    
  //Status
  //  tft.setTextColor(ILI9341_GREENYELLOW);  tft.setTextSize(2);
  //  tft.setCursor(0, 150);
  //  tft.println("Humid:");
  //  tft.setTextColor(ILI9341_CYAN);
  //  tft.setCursor(120, 150);
    //tft.setFont(&FreeSerif12pt7b);
  //  tft.println(myString.substring(22, 27));


    //Mailbox Alert    
   tft.setTextColor(ILI9341_PINK);  tft.setTextSize(2);
   tft.setCursor(0, 160);
    tft.print("Mail: ");
   //tft.setFont(&FreeSerif12pt7b);
    //examine
          if (mail_yes == 1){ //global variable alertState remember ==
            DateTime now_lastMail = rtc.now();
            lastMail = now_lastMail.unixtime(); //last time mail action occured
            Serial.println(" ");
           // Serial.print("Alert state is: ");
           // Serial.println(alertState);
            Serial.print("mail_yes is: ");
            Serial.println(mail_yes);
           // alertState = 0;
            mail_yes = false;
           }//end examine alert string 

//display mail time elapsed in seconds
              //do i need rtc call here?
             DateTime mailcalc = rtc.now();
             mailLastTime = mailcalc.unixtime();
             Serial.print("mailLastTime: ");
             Serial.println(mailLastTime);
             int maillastTimeSeconds = mailLastTime - lastMail;
             Serial.print("mailLastTime - lastMail: ");
             Serial.println(maillastTimeSeconds);
              tft.setCursor(90, 160);
              tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(2);
              tft.print(maillastTimeSeconds);
              tft.print(" secs ago");
//mail happened ago
              
   //RSSI //240 x 320 
    tft.setCursor(240, 230);  //160,310 if portrait mode
    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
    tft.println("RSSI");//RSSI Value
    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
    tft.setCursor(280, 230);  //if portraie mode 200,310
    tft.println(driver.lastRssi());//RSSI Value
    //tft.println("09.99");
   // delay(1000);
} //end displaytextW



/* start of older v52 server code
 *  // V52 rf95 demo RX (SERVER) on M0 - lora feather with TFT ILI9341 Weather shield type display
//adafruit 2.4" tft lcd with touchscreen w/micro sd 
//size 240x320

/*
S: A:0 T:63.82 H:48.88 P:1018.209
Confirmation failed (no ack)
Message From #2, RSSI:-37, S: A:0 T:63.84 H:48.88 P:1018.210
Temp: 63.84
Humid: 48.88
Pressure: 1018.21
Status: 0
56
S: A:0 T:63.84 H:48.88 P:1018.210
Confirmation failed (no ack)
Message From #2, RSSI:-37, S: A:0 T:63.82 H:48.82 P:1018.211
Temp: 63.82
Humid: 48.82
Pressure: 1018.21
Status: 0
50
/

//this seems to time out sometimes

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <SD.h>
#include <Fonts/FreeSerif12pt7b.h>
//need to play with different fonts to customize the display


#ifdef ESP8266
   #define STMPE_CS 16
   #define TFT_CS   0
   #define TFT_DC   15
   #define SD_CS    2
#endif
#ifdef ESP32
   #define STMPE_CS 32
   #define TFT_CS   15
   #define TFT_DC   33
   #define SD_CS    14
#endif

#if defined (__AVR_ATmega32U4__) || defined(ARDUINO_SAMD_FEATHER_M0) || defined (__AVR_ATmega328P__) 
   #define STMPE_CS 6
   #define TFT_CS   9
   #define TFT_DC   10
   #define SD_CS    5
   /************ Radio Setup ***************
    *  
    *
   #define RF95_FREQ 915.0
   #define MY_ADDRESS     1 //server tft display address - my address
   // for feather m0  
   #define RFM95_CS 8
   #define RFM95_RST 4
   #define RFM95_INT 3
   #define TFT_DC 10
   #define TFT_CS 9
   #define SD_CS 5
#endif

#ifdef TEENSYDUINO
   #define TFT_DC   10
   #define TFT_CS   4
   #define STMPE_CS 3
   #define SD_CS    8
#endif
#ifdef ARDUINO_STM32_FEATHER
   #define TFT_DC   PB4
   #define TFT_CS   PA15
   #define STMPE_CS PC7
   #define SD_CS    PC5
#endif
#ifdef ARDUINO_NRF52_FEATHER /* BSP 0.6.5 and higher! *
   #define TFT_DC   11
   #define TFT_CS   31
   #define STMPE_CS 30
   #define SD_CS    27
#endif

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
RH_RF95 rf95(RFM95_CS, RFM95_INT); // Singleton instance of the radio driver
RHReliableDatagram rf95_manager(rf95, MY_ADDRESS); // Class to manage message delivery and receipt, using the driver declared above
int16_t packetnum = 0;  // packet counter, we increment per xmission

//int StatusM = LOW; //keep this state for 12 hours or so
const int ledPin = LED_BUILTIN; //the number of the led pin
int  alertState = LOW; //used to set the mailbox alert state
unsigned long previousMillis = 0; //will store the last time the led was updated
const long interval = 150000; //interval to displag mailbox message

void setup() {
  Serial.begin(115200);
  delay(2000);
  pinMode(LED_BUILTIN, OUTPUT);     
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);//reverse the HIGH to Low for 32u4 - it needs Low - High - Low
  tft.begin();
  Serial.println("Todd FeatherWing TFT Test! v51");
  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
  Serial.println(testFastLines(ILI9341_CYAN, ILI9341_PINK));
  tft.setRotation(2);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_LIGHTGREY);  tft.setTextSize(1);
  tft.print("M0/TFT/LoRA receiver test# ");
  delay(8000);
  tft.fillScreen(ILI9341_BLACK);
  pinMode(RFM95_CS, OUTPUT);
  digitalWrite(RFM95_CS, HIGH);
  // Serial.print("Initializing SD card...");
  //  if (!SD.begin(SD_CS)) {
  // Serial.println("failed!");}
  Serial.println(F("Done!")); 
  digitalWrite(RFM95_RST, LOW);//reverse the HIGH to Low for 32u4 - it needs Low - High - Low
  delay(10);
  digitalWrite(RFM95_RST, HIGH); //reverse the HIGH to Low for 32u4 - it needs Low - High - Low
  delay(10);
  if (!rf95_manager.init()) {
    Serial.println(F("RFM95 radio init failed"));
  
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println(F("setFrequency failed"));
  }
  //seems to work better without the line below
 // rf95.setTxPower(20, true);  // range from 14-20 for power, 
    Serial.print(F("RFM95 radio @"));  Serial.print((int)RF95_FREQ);  Serial.println(F(" MHz"));
} //end setup routine


// Dont put this on the stack:
uint8_t data[] = "Ack, Thank you for the update!";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
// Dont put this on the stack:


void loop(void) {
  //for(uint8_t rotation=0; rotation<4; rotation++) {
  //tft.setRotation(2);
  //testText();
  //delay(1000);
  //Serial.print(F("Horiz/Vert Lines         "));
  //Serial.println(testFastLines(ILI9341_RED, ILI9341_BLUE));
  //delay(4500);
  //getPackets();
  //delay(11500);
     unsigned long currentMillis = millis();
     
     if (rf95_manager.available())// Wait for a message addressed to us from the client
        {
          uint8_t len = sizeof(buf);
          uint8_t from;
              if (rf95_manager.recvfromAck(buf, &len, &from)) 
              {
                    buf[len] = 0; // zero out remaining string
                    Serial.print("Message From #"); Serial.print(from);
                    Serial.print(", RSSI:");Serial.print(rf95.lastRssi());Serial.print(", ");Serial.println((char*)buf);
                    //Serial.println((char*)buf);
                    //Serial.print(buf[14]);Serial.print(buf[15]);Serial.print(".");Serial.print(buf[16]);Serial.println(buf[17]);
                    String myString = (char*)buf;
                    Serial.print("Temp: ");Serial.println(myString.substring(9, 14)); //temperature part of the string
                    Serial.print("Humid: ");Serial.println(myString.substring(17, 22)); //humid part of the string
                    Serial.print("Pressure: ");Serial.println(myString.substring(25, 32)); //pressure part of the string
                    Serial.print("Status: ");Serial.println(myString.substring(5, 6)); //status part of the string
                    
                    Serial.println(buf[21]);
                    Serial.println((char*)buf);//format of char array: "0x2: A:0   T:69.93F       H:53.02% #12"
                  tft.fillScreen(ILI9341_BLACK);//now that an update is available update screen
                      
                      if (myString.substring(5, 6) > "0") {
                      alertState = HIGH;
                      Serial.print("Alert State, setting...");
                         }

                  if (currentMillis - previousMillis >= interval) {
                    //save the last time you blinked the led
                    previousMillis = currentMillis;
                    //if the led is off, turn it on - toggle or not
                       alertState = LOW;
                    }
                                          
                  displayTextW();
                 if (!rf95_manager.sendtoWait(data, sizeof(data), from));// Send a reply back to the originator client
                    Serial.println("Confirmation failed (no ack)"); //what is the purpose of this line?
              }//enf if rf95_manager.recvfromAck
        }//end if rf95_manager.available 
        // displayTextW();
         delay(1000);
  }//end main

unsigned long testText() {
  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_LIGHTGREY);  tft.setTextSize(1);
  tft.print("M0/TFT/LoRA receiver test# ");
  tft.setTextColor(ILI9341_PINK);
  tft.println( 51);
  tft.println();
  tft.setTextColor(ILI9341_GREEN);  //  tft.setTextSize(2);
  tft.println("Hello, Todd");
  tft.println();
  tft.setTextColor(ILI9341_GREENYELLOW);
  tft.setTextSize(1);
  tft.print("Receiving sensor data");
  tft.setTextColor(ILI9341_YELLOW);
  for(uint8_t dots=0; dots<18; dots++) {
  tft.print(".");
  delay(500);
  }
  tft.println( );
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_CYAN);
  tft.print("Reading Humidity,");
  delay(1000);
  tft.setTextColor(ILI9341_GREEN);
  tft.print(" Temperature,");
  tft.setTextColor(ILI9341_MAGENTA);
  delay(1000);
  tft.println(" Pressure");
  tft.setTextColor(ILI9341_ORANGE);
  tft.println("Sending confirmation... ");
  delay(15000);
  return micros() - start;
}//end of testText

unsigned long testFastLines(uint16_t color1, uint16_t color2) {
  unsigned long start;
  int           x, y, w = tft.width(), h = tft.height();
  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for(y=0; y<h; y+=5) tft.drawFastHLine(0, y, w, color1);
  for(x=0; x<w; x+=5) tft.drawFastVLine(x, 0, h, color2);
  return micros() - start;
}//end of testFastLines



void displayTextW()
{
    String myString = (char*)buf;
    tft.setRotation(1);
   // tft.fillScreen(ILI9341_BLACK);
    //bmpDraw("/big/clear.bmp",70,0);
    
    //pressure
    tft.setTextColor(ILI9341_ORANGE);  tft.setTextSize(2);
    tft.setCursor(0, 60);
    tft.println("Pressure:");
    tft.setTextColor(ILI9341_LIGHTGREY);
    tft.setCursor(120, 60);
    //tft.setFont(&FreeSerif12pt7b);
    tft.println(myString.substring(25, 32));
    
    //TEMP   
    tft.setTextColor(ILI9341_GREENYELLOW);  tft.setTextSize(2);
    tft.setCursor(0, 90);
    tft.println("Temp:");
    tft.setTextColor(ILI9341_CYAN);
    tft.setCursor(120, 90);
    //tft.setFont(&FreeSerif12pt7b);
    tft.println(myString.substring(9, 14));

    //HUMID
    tft.setTextColor(ILI9341_ORANGE);  tft.setTextSize(2);
    tft.setCursor(0, 120);
    tft.println("Humid:");
    tft.setTextColor(ILI9341_LIGHTGREY);
    tft.setCursor(120, 120);
    //tft.setFont(&FreeSerif12pt7b);
    tft.println(myString.substring(17, 22));

    //Mailbox Alert    
    tft.setTextColor(ILI9341_PINK);  tft.setTextSize(2);
    tft.setCursor(0, 150);
    tft.println("Mailbox:");
    tft.setTextColor(ILI9341_LIGHTGREY);
    tft.setCursor(120, 150);
    //tft.setFont(&FreeSerif12pt7b);
    tft.println(myString.substring(5, 6));
    //substring examination
    if (alertState==HIGH) {
    tft.setTextColor(ILI9341_LIGHTGREY);  tft.setTextSize(2);
    tft.setCursor(0, 190);
      tft.println("You Have Mail");
    }

   //RSSI //240 x 320 
    tft.setCursor(240, 230);  //160,310 if portrait mode
    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
    tft.println("RSSI");//RSSI Value
    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
    tft.setCursor(280, 230);  //if portraie mode 200,310
    tft.println(rf95.lastRssi());//RSSI Value
    //tft.println("09.99");
   // delay(1000);
} //end displaytextW
© 2018 GitHub, Inc.
Terms
Privacy
Security
Status
Help
Contact GitHub
API
Training
Shop
Blog
*/


