
#pragma once
#include "testPins.h"
#include "testerGfx.h"

/**
 * 
 * @param A
 * @param B
 * @param C
 */
class Component
{
public:
                    Component(TestPin &A, TestPin &B,TestPin &C) : _pA(A),_pB(B),_pC(C)
                    {
                    }
            virtual bool compute()=0;
            virtual bool draw(int yOffset)=0;
            static  void prettyPrint(float value, const char *unit,  char *output);
protected:
            TestPin &_pA, &_pB, &_pC;
};