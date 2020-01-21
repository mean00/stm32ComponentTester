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
};