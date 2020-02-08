# STM32 Component tester

/!\ This is work in progress /!\

This  project is a redone from scratch component tester ( see http://https://www.mikrocontroller.net/articles/AVR-Transistortester ) using a bluepill (STM32F103)

The shopping list is :
  * Bluepill
  * 1.44' SPI 128x128 screen ( [Ebay Link](https://www.ebay.fr/itm/2PCS-1-44-Red-Serial-128X128-SPI-Color-TFT-LCD-Module-Replace-Nokia-5110-LCD/400766556571?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649)
  * LMV324i Quad op Amp (or any rail to rail quad op amp)
  * AMS 3.3v regulator
  * 20k, 470, 330k  1% resistor
  
That's it !

So far only resistor, diode and capacitor are done with no autodection. Accuracy is not bad at all (~ 2%)

Due to the low voltage (3.3v), some mosfet will not work

