//TestPin(int pinNo, int pinADC, int pinVolt int pinDriveHighRes, int pinDriveMed,int pinDriveLow, int hiRes, int medRes,int lowRes);
// Proto 0
#if 0
TestPin   pin1(1,PA2, PA8,  PB5, PB12,PB4,303800,20110,470);  
TestPin   pin2(2,PA1, PA9,  PB7, PB13,PB6,303300,20100,470);
TestPin   pin3(3,PA0, PA10, PB9, PB14,PB8,303000,20130,468);
#else


// Proto 1
//TestPin( No,  ADC,  Volt  High  Med,Low,  hiRes,medRes lowRes);
TestPin   pin1(1,PA2, PB11, PB10, PB2, PB1, 475000,15260,470);  
TestPin   pin2(2,PA1, PB3,  PB5, PB6, PB4, 481000,15150,470);
TestPin   pin3(3,PA0, PB15, PB12, PB14,PB13, 477000,15200,470);


#endif

