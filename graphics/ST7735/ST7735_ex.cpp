#include "SPI.h"
#include "ST7735_ex.h"



#define CS_ON() { *csport &= ~cspinmask;}
#define CS_OFF() {  *csport |= cspinmask;}
#define SEND_DATA() {  *rsport |=  rspinmask;   }

/**
*/
void Adafruit_ST7735Ex::init()
{
    Adafruit_ST7735::initR(); 
    
    // update for our screen
    _width = 128;
    _height = 128;
    rotation = 0;
    
    // There is a garbled line at the bottom, fill it with black
    
    setAddrWindow(0, 128, 128, 129);
    memset(lineBuffer,0,sizeof(lineBuffer));
    CS_ON();
    SEND_DATA();
    writecommand(ST7735_RAMWR); // Row addr set
    SPI.setDataSize (SPI_CR1_DFF_8_BIT); // Set spi 16bit mode  
    SPI.dmaSend(lineBuffer, 128*2, true);
    CS_OFF();    
}
/**
 * 
 * @param CS
 * @param RS
 * @param RST
 */
Adafruit_ST7735Ex::Adafruit_ST7735Ex(int8_t CS, int8_t RS, int8_t RST ) : Adafruit_ST7735(CS,RS,RST)
{
  
  
  
}

/**
 * 
 * @param width
 * @param height
 * @param wx
 * @param wy
 * @param fgcolor
 * @param bgcolor
 * @param data
 */
void Adafruit_ST7735Ex::drawBitmap(int width, int height, int wx, int wy, int fgcolor, int bgcolor, const uint8_t *data)
{
    uint8_t *p=(uint8_t *)data;    
    uint16_t line[320];
    
    width>>=3;
    for(int y=0;y<height;y++)
    {
        uint16_t *o=line;
        setAddrWindow(wx, wy+y, wx+width*8-1, wy+y-1);
        for(int x=0;x<width;x++)
        {
            int stack=*p++;
            for(int step=0;step<8;step++)
            {
                int color;
                if(stack&0x80)                                        
                    color=fgcolor;
                else
                    color=bgcolor;
                *o++=color;
                stack<<=1;
            }            
        }    
        pushColors(line,width*8,0);
    }   
}


/*****************************************************************************/
// Issues 'raw' an array of 16-bit color values to the LCD; used
// externally by BMP examples.  Assumes that setWindowAddr() has
// previously been set to define the bounds.  Max 255 pixels at
// a time (BMP examples read in small chunks due to limited RAM).
/*****************************************************************************/
void Adafruit_ST7735Ex::pushColors(const uint16_t *data, int len, boolean first)
{
  if(first == true) 
  { // Issue GRAM write command only on first call
    writecommand(ST7735_RAMWR); // Row addr set
  }  
  // Send data
   SEND_DATA();
   CS_ON();
   SPI.setDataSize (SPI_CR1_DFF_8_BIT); // Set spi 16bit mode  
   SPI.dmaSend(data, len*2, true);
   CS_OFF();
  
}

/**
 */
void Adafruit_ST7735Ex::drawRLEBitmap(int widthInPixel, int height, int wx, int wy, int fgcolor, int bgcolor, const uint8_t *data)
{
    uint8_t *p=(uint8_t *)data;    
    uint16_t *line=lineBuffer;
    bool first=true;
    
    int widthInByte=(widthInPixel>>3); // pixel -> bytes
    for(int y=0;y<height;y++)
    {
        uint16_t *o=line;
        setAddrWindow(wx, wy+y, wx+widthInPixel, wy+y);
        for(int x=0;x<widthInByte;) // in bytes
        {
            int val=*p++;
            int count=1;
            if(val==0x76)
            {
                val=*p++;
                count=*p++;
            }
            for(int i=0;i<count;i++)
            {
                int stack=val;
                for(int step=0;step<8;step++)
                {
                    int color;
                    if(stack&0x80)                                        
                        color=fgcolor;
                    else
                        color=bgcolor;
                    *o++=color;
                    stack<<=1;
                }            
            }
            x+=count;
        }    
        pushColors(line,widthInPixel,true);
        first=false;
    }   
}

        
  
