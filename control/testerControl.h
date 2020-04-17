#pragma once


#define CONTROL_SHORT 1
#define CONTROL_LONG  2
#define CONTROL_ROTARY  4



class TesterControl
{   
public:
  static void init();
  static void waitForAnyEvent();
  static void test();
  static int  waitForEvent();
  static int  getRotary();
};