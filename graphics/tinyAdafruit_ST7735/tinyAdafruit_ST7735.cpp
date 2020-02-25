
#include "tinyAdafruit_ST7735.h"
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

void Adafruit_ST7735::setRotation(uint8_t x) 
{
  rotation = (x & 3);
  switch (rotation) {
  case 0:
  case 2:
    _width = WIDTH;
    _height = HEIGHT;
    break;
  case 1:
  case 3:
    _width = HEIGHT;
    _height = WIDTH;
    break;
  }
}


/**************************************************************************/
/*!
    @brief Set the font to display when print()ing, either custom or default
    @param  f  The GFXfont object, if NULL use built in 6x8 font
*/
/**************************************************************************/
void Adafruit_ST7735::setFont(const GFXfont *f) {
  if (f) {          // Font struct pointer passed in?
    if (!gfxFont) { // And no current font struct?
      // Switching from classic to new font behavior.
      // Move cursor pos down 6 pixels so it's on baseline.
      cursor_y += 6;
    }
  } else if (gfxFont) { // NULL passed.  Current font struct defined?
    // Switching from new to classic font behavior.
    // Move cursor pos up 6 pixels so it's at top-left of char.
    cursor_y -= 6;
  }
  gfxFont = (GFXfont *)f;
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
#if defined (SPI_HAS_TRANSACTION)
    SPI.beginTransaction(mySPISettings);
#endif
  *rsport |=  rspinmask;
  *csport &= ~cspinmask;

  spiwrite(color >> 8);
  spiwrite(color);

  *csport |= cspinmask;
#if defined (SPI_HAS_TRANSACTION)
    SPI.endTransaction();
#endif
}




// Constructor when using software SPI.  All output pins are configurable.
Adafruit_ST7735::Adafruit_ST7735(int8_t cs, int8_t rs, int8_t sid, int8_t sclk, int8_t rst) 
  
{
  _cs   = cs;
  _rs   = rs;
  _sid  = sid;
  _sclk = sclk;
  _rst  = rst;
  hwSPI = false;
  _width = WIDTH;
  _height = HEIGHT;
  rotation = 0;
  cursor_y = cursor_x = 0;
  textsize_x = textsize_y = 1;
  textcolor = textbgcolor = 0xFFFF;
  wrap = true;
  _cp437 = false;
  gfxFont = NULL;
  
}


// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
Adafruit_ST7735::Adafruit_ST7735(int8_t cs, int8_t rs, int8_t rst) 
{
  _cs   = cs;
  _rs   = rs;
  _rst  = rst;
  hwSPI = true;
  _sid  = _sclk = 0;
   _width = WIDTH;
  _height = HEIGHT;
  rotation = 0;
  cursor_y = cursor_x = 0;
  textsize_x = textsize_y = 1;
  textcolor = textbgcolor = 0xFFFF;
  wrap = true;
  _cp437 = false;
  gfxFont = NULL;
}


void Adafruit_ST7735::writecommand(uint8_t c) {
#if defined (SPI_HAS_TRANSACTION)
  SPI.beginTransaction(mySPISettings);
#endif
  *rsport &= ~rspinmask;
  *csport &= ~cspinmask;

  //Serial.print("C ");
  spiwrite(c);

  *csport |= cspinmask;
#if defined (SPI_HAS_TRANSACTION)
    SPI.endTransaction();
#endif
}


void Adafruit_ST7735::writedata(uint8_t c) {
#if defined (SPI_HAS_TRANSACTION)
    SPI.beginTransaction(mySPISettings);
#endif
  *rsport |=  rspinmask;
  *csport &= ~cspinmask;
    
  //Serial.print("D ");
  spiwrite(c);

  *csport |= cspinmask;
#if defined (SPI_HAS_TRANSACTION)
    SPI.endTransaction();
#endif
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
