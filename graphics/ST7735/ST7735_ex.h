
#include "Adafruit_ST7735.h"
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
        void  myDrawString(const char *st, int padd_up_to_n_pixels=0);
        void  myDrawStringN(const char *st,int length,int padd_up_to_n_pixels=0);
        void  setFontSize(FontSize size);

protected:
        FontInfo          fontInfo[3];
        int                myDrawChar(int x, int y, unsigned char c,  int color, int bg,FontInfo &info);    
        FontInfo          *currentFont;
        void    flood(uint16_t color, uint32_t len);
        void    flood2(uint16_t color, uint16_t bg,uint32_t len);
        int     mySquare(int x, int y, int w, int h, uint16_t filler);
        void    push2Colors(uint8_t *data, int len, boolean first,uint16_t fg, uint16_t bg);


};