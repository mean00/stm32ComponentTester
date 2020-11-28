#ifdef TINY_7735
#include "tinyAdafruit_ST7735.h"
#include "tinyAdafruit_ST7735_priv.h"
#else
#include "Adafruit_ST7735.h"
#endif
#include "MapleFreeRTOS1000_pp.h"
#include "printf.h"

/**
 * Extension to ST7735 to allow block-blit and custom TrueType font
 * It should be fast enough
 */
class Adafruit_ST7735Ex: public Adafruit_ST7735
{
public:
        typedef enum FontSize
        {
            SmallFont,MediumFont,BigFont
        };
        class FontInfo
        {
        public:
          int               maxHeight;          
          int               maxWidth;
          uint16_t         *filler;
          const GFXfont    *font;        
        };

public:
        Adafruit_ST7735Ex(int8_t CS, int8_t RS, int8_t RST = -1);
        void init();
        void drawBitmap(int width, int height, int wx, int wy, int fgcolor, int bgcolor, const uint8_t *data);
        void drawRLEBitmap(int widthInPixel, int height, int wx, int wy, int fgcolor, int bgcolor, const uint8_t *data);
        void pushColors(const uint16_t *data, int len, boolean first);
        
        
        void  setFontFamily(const GFXfont *small, const GFXfont *medium, const GFXfont *big);
        void  setFontSize(FontSize size);
        void  fillScreen(uint16_t color) ;
        void    putPixel(int x,int y, uint16_t color);   
        void  print(const char *z);
        void  print(float f);
        void  print(double d) {return print((float)d);}
        void  print(int f);
protected:
        FontInfo          fontInfo[3];
        
        FontInfo          *currentFont;
        void    flood(uint16_t color, uint32_t len);
        void    push2Colors(uint8_t *data, int len, boolean first,uint16_t fg, uint16_t bg);
        void    pushColors16(const uint16_t *data, int len, boolean first);
        size_t  write(uint8_t c);
        void    myDrawChar(int16_t x, int16_t y, unsigned char c,  uint16_t color, uint16_t bg) ;
        
};
