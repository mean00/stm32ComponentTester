/***************************************************
  This is a library for the Adafruit 1.8" SPI display.

This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
The 1.8" TFT shield
  ----> https://www.adafruit.com/product/802
The 1.44" TFT breakout
  ----> https://www.adafruit.com/product/2088
as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/


#pragma once
#include "Arduino.h"
#include "Print.h"
#include <Adafruit_GFX.h>    // Core graphics library

#include "Adafruit_ST7735_const.h"


class Adafruit_ST7735 : public Adafruit_GFX {

 public:

  Adafruit_ST7735(int8_t CS, int8_t RS, int8_t SID, int8_t SCLK, int8_t RST = -1);
  Adafruit_ST7735(int8_t CS, int8_t RS, int8_t RST = -1);

  void     initB(void),                             // for ST7735B displays
           initR(uint8_t options = INITR_GREENTAB), // for ST7735R
           setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1),
           pushColor(uint16_t color),
           fillScreen(uint16_t color),
           drawPixel(int16_t x, int16_t y, uint16_t color),
           drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color),
           drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color),
           fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
             uint16_t color),
           setRotation(uint8_t r),
           invertDisplay(boolean i);
  uint16_t Color565(uint8_t r, uint8_t g, uint8_t b);


 private:
  uint8_t  tabcolor;

  void     spiwrite(uint8_t),
           writecommand(uint8_t c),
           writedata(uint8_t d),
           commandList(const uint8_t *addr),
           commonInit(const uint8_t *cmdList);


  boolean  hwSPI;
  

  volatile uint32 *dataport, *clkport, *csport, *rsport;
  uint32_t _cs, _rs, _rst, _sid, _sclk,
           datapinmask, clkpinmask, cspinmask, rspinmask,
           colstart, rowstart; // some displays need this changed
  uint16_t lineBuffer[ST7735_TFTHEIGHT_18]; // DMA buffer. 16bit color data per pixel

};
