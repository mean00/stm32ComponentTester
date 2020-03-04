#pragma once


class TesterGfx
{
public:
  static void init();
  static void print(int x, int y, const char *txt);
  static void clear();
  static void drawCapacitor(int offset, const char *value,int pinA, int pinB);
  static void drawResistor(int offset, const char *value,int pinA, int pinB);
  static void drawDiode(int offset, const char *value,int pinA, int pinB);
  static void drawCoil(int offset, const char *value,int pinA, int pinB);
  static void drawPMosFet(const char *line1,const char *line2,  int pinGate, int pinUp, int pinDown);
  static void drawNMosFet(float RdsOn, float Cg, float VfOn, float Vdiode, int pinGate, int pinUp, int pinDown);
  static void printStatus(const char *status);

};