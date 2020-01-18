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

#include "Adafruit_ST7735.h"
#include <limits.h>
#include "pins_arduino.h"
#include "wiring_private.h"
#include <SPI.h>
#include "Adafruit_ST7735_internal.h"

#define SPI_HAS_TRANSACTION 1

inline uint16_t swapcolor(uint16_t x) { 
  return (x << 11) | (x & 0x07E0) | (x >> 11);
}

#if defined (SPI_HAS_TRANSACTION) 
  static SPISettings mySPISettings;
  class spiGuard
  {
  public:      
      spiGuard()
      {
            SPI.beginTransaction(mySPISettings);
      }
      ~spiGuard()
      {
            SPI.endTransaction(); 
      }
  };
#else
   class spiGuard
  {
public:       
      spiGuard()
      {           
      }
      ~spiGuard()
      {       
      }
  };
#endif
  
  

// Constructor when using software SPI.  All output pins are configurable.
Adafruit_ST7735::Adafruit_ST7735(int8_t cs, int8_t rs, int8_t sid, int8_t sclk, int8_t rst) 
  : Adafruit_GFX(ST7735_TFTWIDTH, ST7735_TFTHEIGHT_18)
{
  _cs   = cs;
  _rs   = rs;
  _sid  = sid;
  _sclk = sclk;
  _rst  = rst;
  hwSPI = false;
}


// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
Adafruit_ST7735::Adafruit_ST7735(int8_t cs, int8_t rs, int8_t rst) 
  : Adafruit_GFX(ST7735_TFTWIDTH, ST7735_TFTHEIGHT_18) {
  _cs   = cs;
  _rs   = rs;
  _rst  = rst;
  hwSPI = true;
  _sid  = _sclk = 0;
}

#if defined(CORE_TEENSY) && !defined(__AVR__)
#define __AVR__
#endif

inline void Adafruit_ST7735::spiwrite(uint8_t c) {

  //Serial.println(c, HEX);

  if (hwSPI) {
#if defined (SPI_HAS_TRANSACTION)
      SPI.transfer(c);
#elif defined (__AVR__)
      SPCRbackup = SPCR;
      SPCR = mySPCR;
      SPI.transfer(c);
      SPCR = SPCRbackup;
//      SPDR = c;
//      while(!(SPSR & _BV(SPIF)));
#elif defined (__STM32F1__)
      SPI.write(c);
#elif defined (__arm__)
      SPI.setClockDivider(21); //4MHz
      SPI.setDataMode(SPI_MODE0);
      SPI.transfer(c);
#endif
  } else {
    // Fast SPI bitbang swiped from LPD8806 library
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
      if(c & bit) *dataport |=  datapinmask;
      else        *dataport &= ~datapinmask;
      *clkport |=  clkpinmask;
      *clkport &= ~clkpinmask;
    }
  }
}


void Adafruit_ST7735::writecommand(uint8_t c) {
  spiGuard guard;
  *rsport &= ~rspinmask;
  *csport &= ~cspinmask;

  //Serial.print("C ");
  spiwrite(c);

  *csport |= cspinmask;
}


void Adafruit_ST7735::writedata(uint8_t c) {
  spiGuard guard;  
  *rsport |=  rspinmask;
  *csport &= ~cspinmask;
    
  //Serial.print("D ");
  spiwrite(c);

  *csport |= cspinmask;
}

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Adafruit_ST7735::commandList(const uint8_t *addr) {

  uint8_t  numCommands, numArgs;
  uint16_t ms;

  numCommands = pgm_read_byte(addr++);   // Number of commands to follow
  while(numCommands--) {                 // For each command...
    writecommand(pgm_read_byte(addr++)); //   Read, issue command
    numArgs  = pgm_read_byte(addr++);    //   Number of args to follow
    ms       = numArgs & DELAY;          //   If hibit set, delay follows args
    numArgs &= ~DELAY;                   //   Mask out delay bit
    while(numArgs--) {                   //   For each argument...
      writedata(pgm_read_byte(addr++));  //     Read, issue argument
    }

    if(ms) {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
      if(ms == 255) ms = 500;     // If 255, delay for 500 ms
      delay(ms);
    }
  }
}


// Initialization code common to both 'B' and 'R' type displays
void Adafruit_ST7735::commonInit(const uint8_t *cmdList) {
  colstart  = rowstart = 0; // May be overridden in init func

  pinMode(_rs, OUTPUT);
  pinMode(_cs, OUTPUT);
  csport    = portOutputRegister(digitalPinToPort(_cs));
  rsport    = portOutputRegister(digitalPinToPort(_rs));
  cspinmask = digitalPinToBitMask(_cs);
  rspinmask = digitalPinToBitMask(_rs);

  if(hwSPI) { // Using hardware SPI
#if defined (SPI_HAS_TRANSACTION)
    SPI.begin();
    mySPISettings = SPISettings(8000000, MSBFIRST, SPI_MODE0);
#elif defined (__AVR__)
    SPCRbackup = SPCR;
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV4);
    SPI.setDataMode(SPI_MODE0);
    mySPCR = SPCR; // save our preferred state
    //Serial.print("mySPCR = 0x"); Serial.println(SPCR, HEX);
    SPCR = SPCRbackup;  // then restore
#elif defined (__SAM3X8E__)
    SPI.begin();
    SPI.setClockDivider(21); //4MHz
    SPI.setDataMode(SPI_MODE0);
#elif defined (__STM32F1__)
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
#endif
  } else {
    pinMode(_sclk, OUTPUT);
    pinMode(_sid , OUTPUT);
    clkport     = portOutputRegister(digitalPinToPort(_sclk));
    dataport    = portOutputRegister(digitalPinToPort(_sid));
    clkpinmask  = digitalPinToBitMask(_sclk);
    datapinmask = digitalPinToBitMask(_sid);
    *clkport   &= ~clkpinmask;
    *dataport  &= ~datapinmask;
  }

  // toggle RST low to reset; CS low so it'll listen to us
  *csport &= ~cspinmask;
  if (_rst) {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(500);
    digitalWrite(_rst, LOW);
    delay(500);
    digitalWrite(_rst, HIGH);
    delay(500);
  }

  if(cmdList) commandList(cmdList);
}


// Initialization for ST7735B screens
void Adafruit_ST7735::initB(void) {
  commonInit(Bcmd);
}


// Initialization for ST7735R screens (green or red tabs)
void Adafruit_ST7735::initR(uint8_t options) {
  commonInit(Rcmd1);
  if(options == INITR_GREENTAB) {
    commandList(Rcmd2green);
    colstart = 2;
    rowstart = 1;
  } else if(options == INITR_144GREENTAB) {
    _height = ST7735_TFTHEIGHT_144;
    commandList(Rcmd2green144);
    colstart = 2;
    rowstart = 3;
  } else {
    // colstart, rowstart left at default '0' values
    commandList(Rcmd2red);
  }
  commandList(Rcmd3);

  // if black, change MADCTL color filter
  if (options == INITR_BLACKTAB) {
    writecommand(ST7735_MADCTL);
    writedata(0xC0);
  }

  tabcolor = options;
}


void Adafruit_ST7735::setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1,
 uint8_t y1) {

  if (hwSPI) {
#if defined (__STM32F1__)
    writecommand(ST7735_CASET);
    *rsport |=  rspinmask;
    *csport &= ~cspinmask;
    SPI.setDataSize (SPI_CR1_DFF);
    SPI.write(x0+colstart);
    SPI.write(x1+colstart);
  
    writecommand(ST7735_RASET);
    *rsport |=  rspinmask;
    *csport &= ~cspinmask;
  
    SPI.write(y0+rowstart);
    SPI.write(y1+rowstart);
    SPI.setDataSize(0);
  
    writecommand(ST7735_RAMWR);
 #endif 
  } else {    
    writecommand(ST7735_CASET); // Column addr set
    writedata(0x00);
    writedata(x0+colstart);     // XSTART 
    writedata(0x00);
    writedata(x1+colstart);     // XEND
  
    writecommand(ST7735_RASET); // Row addr set
    writedata(0x00);
    writedata(y0+rowstart);     // YSTART
    writedata(0x00);
    writedata(y1+rowstart);     // YEND
  
    writecommand(ST7735_RAMWR); // write to RAM
  } // end else
}


void Adafruit_ST7735::pushColor(uint16_t color) {
   spiGuard guard;  
  *rsport |=  rspinmask;
  *csport &= ~cspinmask;

  spiwrite(color >> 8);
  spiwrite(color);

  *csport |= cspinmask;
}

void Adafruit_ST7735::drawPixel(int16_t x, int16_t y, uint16_t color) {

  if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;

  setAddrWindow(x,y,x+1,y+1);
   spiGuard guard;  
  *rsport |=  rspinmask;
  *csport &= ~cspinmask;
  
  spiwrite(color >> 8);
  spiwrite(color);

  *csport |= cspinmask;
}


void Adafruit_ST7735::drawFastVLine(int16_t x, int16_t y, int16_t h,
 uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;
  if((y+h-1) >= _height) h = _height-y;
  setAddrWindow(x, y, x, y+h-1);
     spiGuard guard;    
  *rsport |=  rspinmask;
  *csport &= ~cspinmask;

  if (hwSPI) {
#if defined (__STM32F1__)
    SPI.setDataSize (SPI_CR1_DFF); // Set SPI 16bit mode
    lineBuffer[0] = color;
    SPI.dmaSend(lineBuffer, h, 0);
    SPI.setDataSize (0);
#endif
  } else {
    uint8_t hi = color >> 8, lo = color;
    while (h--) {
      spiwrite(hi);
      spiwrite(lo);
    } // end while
  } // end else  
  
  *csport |= cspinmask;
}


void Adafruit_ST7735::drawFastHLine(int16_t x, int16_t y, int16_t w,
  uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;
  if((x+w-1) >= _width)  w = _width-x;
  setAddrWindow(x, y, x+w-1, y);
spiGuard guard;    
  *rsport |=  rspinmask;
  *csport &= ~cspinmask;
  
  if (hwSPI) {
#if defined (__STM32F1__)
    SPI.setDataSize (SPI_CR1_DFF); // Set spi 16bit mode
    lineBuffer[0] = color;
    SPI.dmaSend(lineBuffer, w, 0);
    SPI.setDataSize (0);
#endif
  } else {    
    uint8_t hi = color >> 8, lo = color;
      while (w--) {
      spiwrite(hi);
      spiwrite(lo); 
      }
  } // end else
  *csport |= cspinmask;
}



void Adafruit_ST7735::fillScreen(uint16_t color) {  
  if (hwSPI) {
#if defined (__STM32F1__)
    setAddrWindow(0, 0, _width - 1, _height - 1);
    
    *rsport |=  rspinmask;
    *csport &= ~cspinmask;
    SPI.setDataSize (SPI_CR1_DFF); // Set spi 16bit mode
    lineBuffer[0] = color;
    SPI.dmaSend(lineBuffer, (65535), 0);
    SPI.dmaSend(lineBuffer, ((_width * _height) - 65535), 0);
    SPI.setDataSize (0);  
#endif
  } else {
    fillRect(0, 0,  _width, _height, color);
  } // end else 
}

// fill a rectangle
void Adafruit_ST7735::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
  uint16_t color) {

  // rudimentary clipping (drawChar w/big text requires this)
  if((x >= _width) || (y >= _height)) return;
  if((x + w - 1) >= _width)  w = _width  - x;
  if((y + h - 1) >= _height) h = _height - y;

  setAddrWindow(x, y, x+w-1, y+h-1);

  
    spiGuard guard;    
  *rsport |=  rspinmask;
  *csport &= ~cspinmask;

  if (hwSPI) {
#if defined (__STM32F1__)
    SPI.setDataSize (SPI_CR1_DFF); // Set spi 16bit mode
    lineBuffer[0] = color;
    if (w*h <= 65535) {
    SPI.dmaSend(lineBuffer, (w*h), 0);
    }
    else {
    SPI.dmaSend(lineBuffer, (65535), 0);
    SPI.dmaSend(lineBuffer, ((w*h) - 65535), 0);
    }
    SPI.setDataSize (0);
#endif
  } else {    
    uint8_t hi = color >> 8, lo = color;
    for(y=h; y>0; y--) {
      for(x=w; x>0; x--) {
        spiwrite(hi);
        spiwrite(lo); 
      } // end for
    } // end for
  } // end else
  *csport |= cspinmask;
}


// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t Adafruit_ST7735::Color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


void Adafruit_ST7735::setRotation(uint8_t m) {

  writecommand(ST7735_MADCTL);
  rotation = m % 4; // can't be higher than 3
  switch (rotation) {
   case 0:
     if (tabcolor == INITR_BLACKTAB) {
       writedata(MADCTL_MX | MADCTL_MY | MADCTL_RGB);
     } else {
       writedata(MADCTL_MX | MADCTL_MY | MADCTL_BGR);
     }
     _width  = ST7735_TFTWIDTH;

     if (tabcolor == INITR_144GREENTAB) 
       _height = ST7735_TFTHEIGHT_144;
     else
       _height = ST7735_TFTHEIGHT_18;

     break;
   case 1:
     if (tabcolor == INITR_BLACKTAB) {
       writedata(MADCTL_MY | MADCTL_MV | MADCTL_RGB);
     } else {
       writedata(MADCTL_MY | MADCTL_MV | MADCTL_BGR);
     }

     if (tabcolor == INITR_144GREENTAB) 
       _width = ST7735_TFTHEIGHT_144;
     else
       _width = ST7735_TFTHEIGHT_18;

     _height = ST7735_TFTWIDTH;
     break;
  case 2:
     if (tabcolor == INITR_BLACKTAB) {
       writedata(MADCTL_RGB);
     } else {
       writedata(MADCTL_BGR);
     }
     _width  = ST7735_TFTWIDTH;
     if (tabcolor == INITR_144GREENTAB) 
       _height = ST7735_TFTHEIGHT_144;
     else
       _height = ST7735_TFTHEIGHT_18;

    break;
   case 3:
     if (tabcolor == INITR_BLACKTAB) {
       writedata(MADCTL_MX | MADCTL_MV | MADCTL_RGB);
     } else {
       writedata(MADCTL_MX | MADCTL_MV | MADCTL_BGR);
     }
     if (tabcolor == INITR_144GREENTAB) 
       _width = ST7735_TFTHEIGHT_144;
     else
       _width = ST7735_TFTHEIGHT_18;

     _height = ST7735_TFTWIDTH;
     break;
  }
}


void Adafruit_ST7735::invertDisplay(boolean i) {
  writecommand(i ? ST7735_INVON : ST7735_INVOFF);
}

