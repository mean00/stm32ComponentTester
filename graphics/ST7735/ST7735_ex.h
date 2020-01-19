
#include "Adafruit_ST7735.h"
class Adafruit_ST7735Ex: public Adafruit_ST7735
{

public:
        Adafruit_ST7735Ex(int8_t CS, int8_t RS, int8_t RST = -1);
        void init();
        void drawBitmap(int width, int height, int wx, int wy, int fgcolor, int bgcolor, const uint8_t *data);
        void drawRLEBitmap(int widthInPixel, int height, int wx, int wy, int fgcolor, int bgcolor, const uint8_t *data);
        void pushColors(const uint16_t *data, int len, boolean first);
};