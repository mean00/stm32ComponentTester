
#include "MapleFreeRTOS1000_pp.h"
#pragma once

void myPwm(int pin,int fq); // short cut for pwmGetScaleOverFlow+pwmFromScalerAndOverflow 
void pwmPause(int pin);
void pwmRestart(int pin);
void pwmFromScalerAndOverflow(int pin, int scaler, int overFlow);
void pwmGetScaleOverFlow(int fq,int &scaler, int &ovf);
void pwmGetFrequency(int scaler, int ovf,int &fq);

