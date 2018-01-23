// V50 rf95 demo RX (SERVER) on M0 - lora feather with TFT ILI9341 Weather shield type display
//this seems to time out sometimes
#include <RH_RF95.h>
#include <RHReliableDatagram.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library TFT shield for feather
//not using touch at this time
#include <SPI.h>
#include <SD.h>
#include <Fonts/FreeSerif12pt7b.h>
//need to play with different fonts to customize the display

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

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC); //instatiation of instance of TFT display
RH_RF95 rf95(RFM95_CS, RFM95_INT); // Singleton instance of the radio driver
RHReliableDatagram rf95_manager(rf95, MY_ADDRESS); // Class to manage message delivery and receipt, using the driver declared above

int16_t packetnum = 0;  // packet counter, we increment per xmission

void setup() 
{
  delay(3000); //delay to wait till i am ready to observe
 Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);     
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);//reverse the HIGH to Low for 32u4 - it needs Low - High - Low
  //Serial.println("M0 LoRa Feather Addressed RFM95 RX (server) Test v45 with TFT!");
  tft.begin(); //start the TFT instance
  delay(500);
  pinMode(RFM95_CS, OUTPUT);
  digitalWrite(RFM95_CS, HIGH);
 // Serial.print("Initializing SD card...");
  
  if (!SD.begin(SD_CS)) {
   // Serial.println("failed!");
  }
  
  delay(4000);
 Serial.println("FeatherWing M0 lora ILI9341 TFT Test v50 receiver!.");

// manual reset
  digitalWrite(RFM95_RST, LOW);//reverse the HIGH to Low for 32u4 - it needs Low - High - Low
  delay(10);
  digitalWrite(RFM95_RST, HIGH); //reverse the HIGH to Low for 32u4 - it needs Low - High - Low
  delay(10);
  
  if (!rf95_manager.init()) {
    //Serial.println(F("RFM95 radio init failed"));
    while (1);
  }
  //Serial.println(F("RFM95 radio init OK!"));

  if (!rf95.setFrequency(RF95_FREQ)) {
    //Serial.println(F("setFrequency failed"));
  }

//seems to work better without the line below
 // rf95.setTxPower(20, true);  // range from 14-20 for power, 
    //Serial.print(F("RFM95 radio @"));  Serial.print((int)RF95_FREQ);  Serial.println(F(" MHz"));
} //end setup routine


// Dont put this on the stack:
uint8_t data[] = "Ack, Thank you for the update!";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
// Dont put this on the stack:


void loop() 
  {
 delay(3000);
 //displayWeather();
 getPackets();
 delay(1000);
 displayTextW();


 delay(6000);
 
}//end void loop



void displayTextW()
{
    String myString = (char*)buf;
    tft.setRotation(2);
    tft.fillScreen(ILI9341_BLACK);
    bmpDraw("/big/clear.bmp",70,0);
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
    delay(1000);
} //end displaytextW


#define BUFFPIXEL 20


void bmpDraw(char *filename, int16_t x, int16_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col, x2, y2, bx1, by1;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;
  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
   // Serial.print(F("File not found"));
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature

  Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
   Serial.print(F("Image Offset: ")); 
   Serial.println(bmpImageoffset, DEC);
   Serial.print(F("Header size: "));
   Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
   
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        x2 = x + bmpWidth  - 1; // Lower-right corner
        y2 = y + bmpHeight - 1;
        if((x2 >= 0) && (y2 >= 0)) { // On screen?
          w = bmpWidth; // Width/height of section to load/display
          h = bmpHeight;
          bx1 = by1 = 0; // UL coordinate in BMP file
          if(x < 0) { // Clip left
            bx1 = -x;
            x   = 0;
            w   = x2 + 1;
          }
          if(y < 0) { // Clip top
            by1 = -y;
            y   = 0;
            h   = y2 + 1;
          }
          if(x2 >= tft.width())  w = tft.width()  - x; // Clip right
          if(y2 >= tft.height()) h = tft.height() - y; // Clip bottom
  
          // Set TFT address window to clipped image bounds
          tft.startWrite(); // Requires start/end transaction now
          tft.setAddrWindow(x, y, w, h);
  
          for (row=0; row<h; row++) { // For each scanline...
  
            if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
              pos = bmpImageoffset + (bmpHeight - 1 - (row + by1)) * rowSize;
            else     // Bitmap is stored top-to-bottom
              pos = bmpImageoffset + (row + by1) * rowSize;
            pos += bx1 * 3; // Factor in starting column (bx1)
            if(bmpFile.position() != pos) { // Need seek?
              tft.endWrite(); // End TFT transaction
              bmpFile.seek(pos);
              buffidx = sizeof(sdbuffer); // Force buffer reload
              tft.startWrite(); // Start new TFT transaction
            }
            for (col=0; col<w; col++) { // For each pixel...
              // Time to read more pixel data?
              if (buffidx >= sizeof(sdbuffer)) { // Indeed
                tft.endWrite(); // End TFT transaction
                bmpFile.read(sdbuffer, sizeof(sdbuffer));
                buffidx = 0; // Set index to beginning
                tft.startWrite(); // Start new TFT transaction
              }
              // Convert pixel from BMP to TFT format, push to display
              b = sdbuffer[buffidx++];
              g = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];
              tft.writePixel(tft.color565(r,g,b));
            } // end pixel
          } // end scanline
          tft.endWrite(); // End last TFT transaction
        } // end onscreen;
      } // end goodBmp
    }
  }

  bmpFile.close();
 // if(!goodBmp) Serial.println(F("BMP format not recognized."));
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

unsigned long testFilledRoundRects() {
  unsigned long start;
  int           i, i2,
                cx = tft.width() / 2 - 1,
                cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for(i=min(tft.width(), tft.height()); i>10; i-=6) { // for(i=min(tft.width(), tft.height()); i>20; i-=6) { 
    i2 = i / 2;
    tft.fillRoundRect(cx-i2, cy-i2, i, i, i/8, tft.color565(0, i, 0));
    yield();
  }

  return micros() - start;
}

void getPackets()
{
        if (rf95_manager.available())// Wait for a message addressed to us from the client
        {
          uint8_t len = sizeof(buf);
          uint8_t from;
              if (rf95_manager.recvfromAck(buf, &len, &from)) 
              {
                    buf[len] = 0; // zero out remaining string
                  //  Serial.print("Message From #"); Serial.print(from);
                   // Serial.print(", RSSI:");Serial.print(rf95.lastRssi());Serial.print(", ");Serial.println((char*)buf);
//Serial.println((char*)buf);
 //Serial.print(buf[14]);Serial.print(buf[15]);Serial.print(".");Serial.print(buf[16]);Serial.println(buf[17]);
 //String myString = (char*)buf;
 //Serial.print("Temp: ");Serial.println(myString.substring(14, 19)); //temperature part of the string
 //Serial.print("Humid: ");Serial.println(myString.substring(22, 27)); //temperature part of the string
// Serial.print("Status: ");Serial.println(myString.substring(10, 11)); //temperature part of the string
 //Serial.println(buf[21]);
                    //Serial.println((char*)buf);//format of char array: "0x2: A:0   T:69.93F       H:53.02% #12"
                    
                    //Blink(LED_BUILTIN, 40, 3); //blink LED 3 times, 40ms between blinks
              if (!rf95_manager.sendtoWait(data, sizeof(data), from));// Send a reply back to the originator client
                   // Serial.println("Sending failed (no ack)");
              
                  
                  //int a=((buf[6] - 48)*10) + (buf[7] - 48);
                  //int b=((buf[9] - 48)*10) + (buf[10] -48);
                  //float c=b;
                  //Asci4.setBrightness(3);
                  //Asci4.println(c/100+a); //display temp as float nn.nn
                  //Asci4.writeDisplay();
                                    
              }//enf if rf95_manager.recvfromAck
        }//end if rf95_manager.available 
}//end of getpacket
  
