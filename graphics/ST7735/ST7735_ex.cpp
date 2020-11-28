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
 * @param r
 * @param g
 * @param b
 * @return 
 */
uint16_t Adafruit_ST7735::Color565(uint8_t r, uint8_t g, uint8_t b) {
   return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
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
void Adafruit_ST7735Ex::pushColors16(const uint16_t *data, int len, boolean first)
{
  if(first == true) 
  { // Issue GRAM write command only on first call
    writecommand(ST7735_RAMWR); // Row addr set
  }  
  // Send data
   SEND_DATA();
   CS_ON();
   SPI.setDataSize (SPI_CR1_DFF_16_BIT); // Set spi 16bit mode  
   SPI.dmaSend(data, len, true);
   CS_OFF();
  
}
/**
 * 
 * @param widthInPixel
 * @param height
 * @param wx
 * @param wy
 * @param fgcolor
 * @param bgcolor
 * @param data
 */
void Adafruit_ST7735Ex::drawRLEBitmap(int widthInPixel, int height, int wx, int wy, int fgcolor, int bgcolor, const uint8_t *data)
{    
    uint16_t *line=lineBuffer;
    bool first=true;
    int nbPixel=widthInPixel*height;
    int pixel=0;
    setAddrWindow(wx, wy, wx+widthInPixel-1, wy+height);
    int mask=0;
    int cur;   
    uint16_t *o=line;
    int ready=0;
    int repeat;
    uint16_t color;
    while(pixel<nbPixel)        
    {
        // load next
        cur=*data++;
        if(cur==0x76)
        {
            cur=*data++;
            repeat=*data++;
        }else
        {
            repeat=1;
        }
        // 8 pixels at a time
        for(int r=0;r<repeat;r++)
        {
            int mask=0x80;
            for(int i=0;i<8;i++)
            {
                if(mask & cur)
                {
                    color=fgcolor;
                }else
                    color=0xff00*0+1*bgcolor;
                mask>>=1;
                *o++=color;
                ready++;
            }
            if(ready>(ST7735_BUFFER_SIZE-16))
            { // Flush
              pushColors16(line,ready,first);  
              first=false;
              ready=0;
              o=line;
            }
        }
        pixel+=repeat*8;
    }
    pushColors16(line,ready,true);  
}

/**
 * 
 * @param size
 */
void  Adafruit_ST7735Ex::setFontSize(FontSize size)
{
    switch(size)
    {
        case SmallFont :  currentFont=fontInfo+0;break;
        default:
        case MediumFont :   currentFont=fontInfo+1;break;
        case BigFont :   currentFont=fontInfo+2;break;
    }    
    gfxFont=currentFont->font;
}
/**
 * \fn checkFont
 * \brief extract max width/ max height from the font
 */
static void checkFont(const GFXfont *font, Adafruit_ST7735Ex::FontInfo *info)
{
    int mW=0,mH=0;
    int x,y;
   for(int i=font->first;i<font->last;i++)
   {
         GFXglyph *glyph  = font->glyph+i-font->first;
         x=glyph->xAdvance;
         y=-glyph->yOffset;
         if(x>mW) mW=x;         
         if(y>mH) mH=y;
   }
    info->maxHeight=mH + 1;
    info->maxWidth=mW;    
    info->font=font;
}

/**
 * 
 * @param small
 * @param medium
 * @param big
 */
void  Adafruit_ST7735Ex::setFontFamily(const GFXfont *small, const GFXfont *medium, const GFXfont *big)
{
    checkFont(small, fontInfo+0);
    checkFont(medium,fontInfo+1);
    checkFont(big,   fontInfo+2);
}        
/**
 * 
 * @param color
 */
void Adafruit_ST7735Ex::fillScreen(uint16_t color) 
{  

    setAddrWindow(0, 0, _width - 1, _height - 1);    
    flood(color,_width*_height);
}

/**
 * 
 * @param color
 * @param len
 */  
void    Adafruit_ST7735Ex::flood(uint16_t color, uint32_t len)
{
    SEND_DATA();
    CS_ON();    
    SPI.setDataSize (SPI_CR1_DFF); // Set spi 16bit mode
    lineBuffer[0] = color;
    xAssert(len<65535);
    SPI.dmaSend(lineBuffer, len, 0);
    SPI.setDataSize (0);
    CS_OFF();
}
/**
 * 
 * @param data
 * @param len
 * @param first
 * @param fg
 * @param bg
 */
void    Adafruit_ST7735Ex::push2Colors(uint8_t *data, int len, boolean first,uint16_t fg, uint16_t bg)
{
    xAssert(len<ST7735_TFTHEIGHT_18);
    uint16_t *p=lineBuffer;
    for(int i=0;i<len;i++)
    {
        if(data[i])
            p[i]=fg;
        else
            p[i]=bg;
    }
   SEND_DATA();
   CS_ON();
   SPI.setDataSize (0*SPI_CR1_DFF_16_BIT); // Set spi 16bit mode  
   SPI.dmaSend(lineBuffer, len*2, true);
   CS_OFF();
}

// hooks
static GFXglyph *pgm_read_glyph_ptr(const GFXfont *gfxFont, uint8_t c) {  return gfxFont->glyph + c;}
static  uint8_t *pgm_read_bitmap_ptr(const GFXfont *gfxFont) {  return gfxFont->bitmap;}

/**
 * 
 * @param x
 * @param y
 * @param c
 * @param color
 * @param bg
 */
void    Adafruit_ST7735Ex::putPixel(int x, int y, uint16_t c)
{
     setAddrWindow(x,y,x+1,  y+1);
     writecommand(ST7735_RAMWR); // Row addr set
     // Send data
     SEND_DATA();
     CS_ON();
     SPI.setDataSize (SPI_CR1_DFF_16_BIT); // Set spi 16bit mode  
     SPI.dmaSend(&c, 1, true);
     CS_OFF();
    
}

/**
 * 
 * @param x
 * @param y
 * @param c
 * @param color
 * @param bg
 */
void Adafruit_ST7735Ex::myDrawChar(int16_t x, int16_t y, unsigned char c,  uint16_t color, uint16_t bg) 
{
    int cr=c;
    cr -= gfxFont->first;
    GFXglyph *glyph =  gfxFont->glyph + cr;
    uint8_t *bitmap = gfxFont->bitmap;

    int bo = glyph->bitmapOffset;
    int w = glyph->width;
    int h = glyph->height;
    x+=glyph->xOffset;
    y+=glyph->yOffset;
   
    #define OFFSET -1
    bool first=true;
    int dex=0;
    setAddrWindow(x,y,   x+w+OFFSET,y+h+OFFSET);
    int bits = 0, bit = 0;
    int n=h*w;
    int mask=0;
    uint8_t *data=bitmap+bo;
    for (int i=n-1;i>=0;i--) 
    {      
        if (!mask) 
        {
          bits = *data++;
          if(1 && dex<(ST7735_BUFFER_SIZE-16) && i>7)
          {
              uint16_t *p=lineBuffer+dex;
              switch(bits)
              {
              case 0:
                *p++=bg;*p++=bg;*p++=bg;*p++=bg;
                *p++=bg;*p++=bg;*p++=bg;*p++=bg;
                dex+=8;
                i-=7;
                continue;                 
                break;
              
              case 0xff:
                *p++=color;*p++=color;*p++=color;*p++=color;
                *p++=color;*p++=color;*p++=color;*p++=color;
                dex+=8;
                i-=7;
                continue;                 
                break;
              case 0x0f:
                *p++=bg;*p++=bg;*p++=bg;*p++=bg;
                *p++=color;*p++=color;*p++=color;*p++=color; // compiler is smart enough to optimize this
                dex+=8;
                i-=7;
                continue;                                   
                break;
              case 0xF0:
                *p++=color;*p++=color;*p++=color;*p++=color;
                *p++=bg;*p++=bg;*p++=bg;*p++=bg;
                dex+=8;
                i-=7;
                continue;                                   
                break;
             default:
                break;       
            }
          }
          mask=0x80;
        }
        if (bits & mask) 
        {          
            lineBuffer[dex]=color;          
        }else
        {
            lineBuffer[dex]=bg;          
        }
        mask>>=1;
        dex++;
        if(dex==ST7735_BUFFER_SIZE)
        {
            pushColors16(lineBuffer,dex,first);
            first=false;
            dex=0;
        }
    }
    if(dex)
        pushColors16(lineBuffer,dex,first);
    return;
}

/**
 * 
 * @param z
 */
void  Adafruit_ST7735Ex::print(const char *z)
{
   int l=strlen(z);
   while(*z)
   {
       int inc=write(*z);
       cursor_x+=inc;
       z++;
   }
}
void  Adafruit_ST7735Ex::print(float f)
{
    char st[50];
    sprintf(st,"%f",f);
    print(st);
}
   void  Adafruit_ST7735Ex::print(int f)
{
    char st[50];
    sprintf(st,"%d",f);
    print(st);
}     
/**
 * 
 * @param c
 * @return 
 */
size_t Adafruit_ST7735Ex::write(uint8_t c) 
{
#if 0
  if (!gfxFont)
    { return Adafruit_ST7735::write(c);}// 'Classic' built-in font
  if((textsize_x!=1 ) || (textsize_y!=1))  
    { return Adafruit_ST7735::write(c);}
#endif
    xAssert(gfxFont);
    xAssert(textsize_x==1 && textsize_y==1);
  
    if (c == '\n') 
    {
      cursor_x = 0;
      cursor_y +=          textsize_y * gfxFont->yAdvance;
      return 1;
    } 
    if(c=='\r')
      return 1;
    uint8_t first = gfxFont->first;
    if ((c < first) || (c > gfxFont->last)) 
        return 1;
    
    GFXglyph *glyph = gfxFont->glyph + c-first;
    int w = glyph->width,   h = glyph->height;
    if ((w <= 0) || (h <= 0)) 
    {
        cursor_x += glyph->xAdvance ;    
        return 1;
    }

    int xo = glyph->xOffset; // sic
    if (wrap && ((cursor_x +  (xo + w)) > _width)) 
    {
      cursor_x = 0;
      cursor_y +=   gfxFont->yAdvance;
    }
#if 0        
        drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x,  textsize_y);
#else
        // this one is about 10 times faster
        myDrawChar(cursor_x, cursor_y, c, textcolor, textbgcolor); 
#endif
      
    cursor_x += glyph->xAdvance ;    
    return 1;
}