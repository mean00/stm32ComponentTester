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
  static void drawPMosFet(float RdsOn, float Cg, float VfOn, float Vdiode, int pinGate, int pinUp, int pinDown);
  static void drawNMosFet(float RdsOn, float Cg, float VfOn, float Vdiode, int pinGate, int pinUp, int pinDown);
  static void drawNPN(float hfe, float vf,int base, int emitter,int collector);
  static void drawPNP(float hfe, float vf,int base, int emitter,int collector);
  static void printStatus(const char *status);

};