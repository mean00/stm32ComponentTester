#pragma once

#include "Print.h"
#include "../Adafruit-GFX/gfxfont.h"

#define INITR_GREENTAB 0x0
#define INITR_REDTAB   0x1
#define INITR_BLACKTAB   0x2
#define INITR_GREENTAB 0x0
#define INITR_REDTAB   0x1
#define INITR_BLACKTAB   0x2

#define ST7735_TFTWIDTH  128
// for 1.44" display
#define ST7735_TFTHEIGHT_144 128
// for 1.8" display
#define ST7735_TFTHEIGHT_18  160

#define INITR_18GREENTAB    INITR_GREENTAB
#define INITR_18REDTAB      INITR_REDTAB
#define INITR_18BLACKTAB    INITR_BLACKTAB
#define INITR_144GREENTAB   0x1

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

                Adafruit_ST7735(int8_t CS, int8_t RS, int8_t RST = -1);
        void   initR(uint8_t options = INITR_GREENTAB); // for ST7735R
        void   setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
        void   pushColor(uint16_t color);
        void   fillScreen(uint16_t color);
        void   drawPixel(int16_t x, int16_t y, uint16_t color);
        void   drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
        void   drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
        void   fillRect(int16_t x, int16_t y, int16_t w, int16_t h,   uint16_t color);
        void   setRotation(uint8_t r);
        void   invertDisplay(boolean i);
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

 
    void     spiwrite(uint8_t);
    void     writecommand(uint8_t c);
    void     writedata(uint8_t d);
    void     commandList(const uint8_t *addr);
    void     commonInit(const uint8_t *cmdList);

  boolean  hwSPI;
  uint8_t  tabcolor;


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
  boolean wrap;             ///< If set, 'wrap' text at right edge of display  ///< If set, use correct CP437 charset (default is off)
  GFXfont           *gfxFont;   ///< Pointer to special font  
};