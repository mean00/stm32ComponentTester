#pragma once
#include "testPins.h"
#include "Component.h"
#include "Mosfet.h"
/**
 */
class PMosFet : public  Mosfet
{
public:                 
                    // Order is Gate, Drain Source
                    PMosFet( TestPin &A, TestPin &B,TestPin &C) :  Mosfet(A,B,C)
                    {
                  
                    }
            virtual bool compute()            ;
            virtual bool draw2(int yOffset);
                    
            
protected:                      
            bool  computeDiode();
            bool  computeRdsOn();
            bool  computeVgOn();
            bool  computeCg();
public:
            
};
