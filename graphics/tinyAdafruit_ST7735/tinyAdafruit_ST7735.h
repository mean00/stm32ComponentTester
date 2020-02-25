#pragma once

#include "Print.h"
#include "../Adafruit-GFX/gfxfont.h"
#include "tinyAdafruit_ST7735_priv.h"
/**
 * 
 * @param CS
 * @param RS
 * @param SID
 * @param SCLK
 * @param RST
 */
class Adafruit_ST7735 : public Print
{
  
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

    void setCursor(int16_t x, int16_t y) 
    {
            cursor_x = x;
            cursor_y = y;
    }
    void setTextSize(uint8_t s), setTextSize(uint8_t sx, uint8_t sy);
    void setFont(const GFXfont *f = NULL);
    void setTextColor(uint16_t c, uint16_t bg) 
    {
        textcolor = c;
        textbgcolor = bg;
    }

 public:
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
  
  int WIDTH,        HEIGHT;            ///< This is the 'raw' display width - never changes  ///< This is the 'raw' display height - never changes
  int _width,      _height;         
  int  cursor_x,   cursor_y;     ///< Display width as modified by current rotation ///< Display height as modified by current rotation  ///< x location to start print()ing text  ///< y location to start print()ing text
  int textcolor,   textbgcolor;   ///< 16-bit background color for print()  ///< 16-bit text color for print()
  int textsize_x,  textsize_y;  ///< Desired magnification in X-axis of text to print()   ///< Desired magnification in Y-axis of text to print()
  int rotation;       ///< Display rotation (0 thru 3)
  boolean wrap,    _cp437;             ///< If set, 'wrap' text at right edge of display  ///< If set, use correct CP437 charset (default is off)
  GFXfont           *gfxFont;   ///< Pointer to special font
  
};