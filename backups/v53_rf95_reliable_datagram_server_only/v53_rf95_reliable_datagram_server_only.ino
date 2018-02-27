#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

#define CLIENT_ADDRESS 1 //the 32u4 is the client/peripheral with better power saving
#define SERVER_ADDRESS 2 //the M0 is the server, like the gateway
#define RF95_FREQ 915.0
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
#define TFT_DC 10
#define TFT_CS 9
#define SD_CS 5

RH_RF95 driver(RFM95_CS,RFM95_INT);
RHReliableDatagram manager(driver, SERVER_ADDRESS);


void setup() 
{


  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
    Serial.println("init failed");
    delay(1000);
   // driver.setFrequency(RF95_FREQ);  //after init set freq just to be certain.
delay(1000);
Serial.println("V55 M0 zero server 0x2 test");
Serial.print(F("RFM95 Server radio @"));  Serial.print((int)RF95_FREQ);  Serial.println(F(" MHz"));
delay(500);
Serial.println(driver.printRegisters());




  /*
  
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
  tft.print("V53 M0/TFT/LoRA receiver test# ");
  delay(8000);
  tft.fillScreen(ILI9341_BLACK);
*/

} //end setup 

uint8_t data[] = "Hello litle 32u4 from v54 M0 server";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
  if (manager.available())
  {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from))
    {
      Serial.print("got request from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);

      // Send a reply back to the originator client
      if (!manager.sendtoWait(data, sizeof(data), from))
        Serial.println("sendtoWait failed");
    }
  }
}//end main loop


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
    tft.println(rf95_driver.lastRssi());//RSSI Value
    //tft.println("09.99");
   // delay(1000);
} //end displaytextW
*/

