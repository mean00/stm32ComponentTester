#pragma once
class Component;
enum COMPONENT_TYPE
{
  COMPONENT_OPEN=0,
  COMPONENT_RESISTOR=1,
  COMPONENT_CAPACITOR=2,
  COMPONENT_DIODE=3,
};
Component *identity(TestPin &A, TestPin &B, TestPin &C,COMPONENT_TYPE &type);
