#pragma once

#define RGB565(r,g,b)  (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

class TesterGfx
{
public:
  static void init();
  static void splash();
  static void print(int x, int y, const char *txt);
  static void printSmall(int x, int y, const char *txt);
  static void clear();
  static void drawCapacitor(int offset, const char *value,int pinA, int pinB);
  static void drawResistor(int offset, const char *value,int pinA, int pinB);
  static void drawDiode(int offset, const char *value,int pinA, int pinB);
  static void drawCoil(int offset, const char *value,int pinA, int pinB);
  static void drawPMosFet( int pinGate, int pinUp, int pinDown);
  static void drawNMosFet( int pinGate, int pinUp, int pinDown);
  static void drawMosInfo(int page,float RdsOn, float Cg, float VfOn, float Vdiode);  
  static void drawNPN(float hfe, float vf,int base, int emitter,int collector);
  static void drawPNP(float hfe, float vf,int base, int emitter,int collector);
  static void drawZif();
  static void drawCurve(int nb, uint16_t *data);
  static void test(void);
  static void title(const char *);
  static void bottomLine(const char *x);
  static void topLine(const char *x);
  static void progress6(int pg);
  static void highlight(bool onoff);
};