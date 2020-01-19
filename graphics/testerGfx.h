#pragma once


class TesterGfx
{
public:
  static void init();
  static void print(int x, int y, const char *txt);
  static void clear();
  static void drawCapacitor(int offset, const char *value,int pinA, int pinB);
  
};