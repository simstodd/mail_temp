// ILI9341 example with embedded color bitmaps in sketch.
// WILL NOT FIT ON ARDUINO UNO OR OTHER AVR BOARDS;
// uses large bitmap image stored in array!

// Options for converting images to the format used here include:
//   http://www.rinkydinkelectronics.com/t_imageconverter565.php
// or
//  GIMP (https://www.gimp.org/) as follows:
//    1. File -> Export As
//    2. In Export Image dialog, use 'C source code (*.c)' as filetype.
//    3. Press export to get the export options dialog.
//    4. Type the desired variable name into the 'prefixed name' box.
//    5. Uncheck 'GLIB types (guint8*)'
//    6. Check 'Save as RGB565 (16-bit)'
//    7. Press export to save your image.
//  Assuming 'image_name' was typed in the 'prefixed name' box of step 4,
//  you can have to include the c file, then using the image can be done with:
//    tft.drawRGBBitmap(0, 0, image_name.pixel_data, image_name.width, image_name.height);
//  See also https://forum.pjrc.com/threads/35575-Export-for-ILI9341_t3-with-GIMP

#include "SPI.h"
#include <Adafruit_ILI9341.h>
#include "dragon.h"
#include "dragon2.h"
#include "ALL_Icons.h"

// For the Adafruit shield, these are the default.
//#define TFT_DC 9
//#define TFT_CS 10

// Feather 32u4 or M0 with TFT FeatherWing:
#define TFT_DC 10
#define TFT_CS  9

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

void setup() {
  tft.begin();
}

void loop(void) {

    tft.setRotation(2);
    tft.fillScreen(ILI9341_BLACK);
    //var.drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
    tft.drawRGBBitmap(random(-DRAGON_WIDTH , tft.width()),random(-DRAGON_HEIGHT, tft.height()),(uint16_t *)dragonBitmap2,DRAGON_WIDTH, DRAGON_HEIGHT);
    delay(3000);
  tft.drawRGBBitmap(random(-L0_WIDTH , tft.width()),random(-L0_HEIGHT, tft.height()),(uint16_t *)L0,L0_WIDTH, L0_HEIGHT);
  tft.drawRGBBitmap(random(-L1_WIDTH , tft.width()),random(-L1_HEIGHT, tft.height()),(uint16_t *)L1,L1_WIDTH, L1_HEIGHT);
  tft.drawRGBBitmap(random(-L2_WIDTH , tft.width()),random(-L2_HEIGHT, tft.height()),(uint16_t *)L2,L2_WIDTH, L2_HEIGHT);
  tft.drawRGBBitmap(random(-L3_WIDTH , tft.width()),random(-L3_HEIGHT, tft.height()),(uint16_t *)L3,L3_WIDTH, L3_HEIGHT);
  tft.drawRGBBitmap(random(-L4_WIDTH , tft.width()),random(-L4_HEIGHT, tft.height()),(uint16_t *)L4,L4_WIDTH, L4_HEIGHT);
  tft.drawRGBBitmap(random(-L5_WIDTH , tft.width()),random(-L5_HEIGHT, tft.height()),(uint16_t *)L5,L5_WIDTH, L5_HEIGHT);
  tft.drawRGBBitmap(random(-L6_WIDTH , tft.width()),random(-L6_HEIGHT, tft.height()),(uint16_t *)L6,L6_WIDTH, L6_HEIGHT);
  tft.drawRGBBitmap(random(-L7_WIDTH , tft.width()),random(-L7_HEIGHT, tft.height()),(uint16_t *)L7,L7_WIDTH, L7_HEIGHT);
  tft.drawRGBBitmap(random(-L8_WIDTH , tft.width()),random(-L8_HEIGHT, tft.height()),(uint16_t *)L8,L8_WIDTH, L8_HEIGHT);
  tft.drawRGBBitmap(random(-L9_WIDTH , tft.width()),random(-L9_HEIGHT, tft.height()),(uint16_t *)L9,L9_WIDTH, L9_HEIGHT);
  tft.drawRGBBitmap(random(-L10_WIDTH , tft.width()),random(-L10_HEIGHT, tft.height()),(uint16_t *)L10,L10_WIDTH, L10_HEIGHT);
  tft.drawRGBBitmap(random(-L11_WIDTH , tft.width()),random(-L11_HEIGHT, tft.height()),(uint16_t *)L11,L11_WIDTH, L11_HEIGHT);
  tft.drawRGBBitmap(random(-L12_WIDTH , tft.width()),random(-L12_HEIGHT, tft.height()),(uint16_t *)L12,L12_WIDTH, L12_HEIGHT);
  tft.drawRGBBitmap(random(-L13_WIDTH , tft.width()),random(-L13_HEIGHT, tft.height()),(uint16_t *)L13,L13_WIDTH, L13_HEIGHT);
  tft.drawRGBBitmap(random(-L14_WIDTH , tft.width()),random(-L14_HEIGHT, tft.height()),(uint16_t *)L14,L14_WIDTH, L14_HEIGHT);
  tft.drawRGBBitmap(random(-L15_WIDTH , tft.width()),random(-L15_HEIGHT, tft.height()),(uint16_t *)L15,L15_WIDTH, L15_HEIGHT);
  tft.drawRGBBitmap(random(-L16_WIDTH , tft.width()),random(-L16_HEIGHT, tft.height()),(uint16_t *)L16,L16_WIDTH, L16_HEIGHT);
  tft.drawRGBBitmap(random(-L17_WIDTH , tft.width()),random(-L17_HEIGHT, tft.height()),(uint16_t *)L17,L17_WIDTH, L17_HEIGHT);
  tft.drawRGBBitmap(random(-L18_WIDTH , tft.width()),random(-L18_HEIGHT, tft.height()),(uint16_t *)L18,L18_WIDTH, L18_HEIGHT);
  tft.drawRGBBitmap(random(-L19_WIDTH , tft.width()),random(-L19_HEIGHT, tft.height()),(uint16_t *)L19,L19_WIDTH, L19_HEIGHT);
  tft.drawRGBBitmap(random(-L20_WIDTH , tft.width()),random(-L20_HEIGHT, tft.height()),(uint16_t *)L20,L20_WIDTH, L20_HEIGHT);
  tft.drawRGBBitmap(random(-L21_WIDTH , tft.width()),random(-L21_HEIGHT, tft.height()),(uint16_t *)L21,L21_WIDTH, L21_HEIGHT);
  tft.drawRGBBitmap(random(-L22_WIDTH , tft.width()),random(-L22_HEIGHT, tft.height()),(uint16_t *)L22,L22_WIDTH, L22_HEIGHT);
  tft.drawRGBBitmap(random(-L23_WIDTH , tft.width()),random(-L23_HEIGHT, tft.height()),(uint16_t *)L23,L23_WIDTH, L23_HEIGHT);
  tft.drawRGBBitmap(random(-LE_WIDTH , tft.width()),random(-LE_HEIGHT, tft.height()),(uint16_t *)LE,LE_WIDTH, LE_HEIGHT);
  delay(3000);
  
}
