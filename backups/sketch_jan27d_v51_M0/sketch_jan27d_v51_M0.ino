// V51 rf95 demo RX (SERVER) on M0 - lora feather with TFT ILI9341 Weather shield type display
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
   /************ Radio Setup ***************/
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
#ifdef ARDUINO_NRF52_FEATHER /* BSP 0.6.5 and higher! */
   #define TFT_DC   11
   #define TFT_CS   31
   #define STMPE_CS 30
   #define SD_CS    27
#endif

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
RH_RF95 rf95(RFM95_CS, RFM95_INT); // Singleton instance of the radio driver
RHReliableDatagram rf95_manager(rf95, MY_ADDRESS); // Class to manage message delivery and receipt, using the driver declared above
int16_t packetnum = 0;  // packet counter, we increment per xmission


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
    while (1);
  }
  Serial.println(F("RFM95 radio init OK!"));

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
                  displayTextW();
                 if (!rf95_manager.sendtoWait(data, sizeof(data), from));// Send a reply back to the originator client
                    Serial.println("Confirmation failed (no ack)");
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
    tft.setRotation(2);
   // tft.fillScreen(ILI9341_BLACK);
    //bmpDraw("/big/clear.bmp",70,0);
    tft.setTextColor(ILI9341_YELLOW);  tft.setTextSize(2);
    tft.setCursor(0, 90);
    tft.println("Current Temp:");
    tft.setTextColor(ILI9341_CYAN);
    tft.setCursor(170, 90);
    //tft.setFont(&FreeSerif12pt7b);
    tft.println(myString.substring(14, 19));
     //tft.println("88.89");
    tft.setTextColor(ILI9341_BLUE);  tft.setTextSize(2);
    tft.setCursor(0, 120);
    tft.println("Current Humid:");
    tft.setTextColor(ILI9341_RED);
    tft.setCursor(170, 120);
    //tft.setFont(&FreeSerif12pt7b);
    tft.println(myString.substring(22, 27));
    //tft.println("88.89");  
    tft.setCursor(160, 310);  
    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
    tft.println("RSSI");//RSSI Value
    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
    tft.setCursor(200, 310);  
    tft.println(rf95.lastRssi());//RSSI Value
    //tft.println("09.99");
   // delay(1000);
} //end displaytextW
