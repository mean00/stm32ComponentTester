
#pragma once
#include "testPins.h"
#include "Component.h"

class Mosfet : public Component
{
public:
    Mosfet(TestPin &A, TestPin &B,TestPin &C) :  Component(A,B,C)
    {
      
    }
    bool computeDiode(TestPin &top, TestPin &bottom,float &value);
};