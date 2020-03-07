# STM32 Component tester

## Scope

/!\ This is work in progress, it is not usable at the moment /!\

This  project is a redone from scratch component tester 
( see http://https://www.mikrocontroller.net/articles/AVR-Transistortester ).

The tester is powered by a bluepill (STM32F103 or GD32F103 based)

The shopping list is :
  * Bluepill
  * 1.44' SPI 128x128 screen ( [Ebay Link](https://www.ebay.fr/itm/2PCS-1-44-Red-Serial-128X128-SPI-Color-TFT-LCD-Module-Replace-Nokia-5110-LCD/400766556571?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649 ))
  * LMV324i Quad op Amp (or any rail to rail quad op amp)
  * AMS 3.3v voltage regulator regulator
  * 20k, 470, 330k  1% resistor
  
That's it !

Due to the low voltage (3.3v), some mosfet will not work

## How to build

* Install arduino and cmake
* Checkout the project, make sure you checkout the submodules
* Edit platformConfig.cmake to set the path to your compiler (i strongly suggest to use that ( [one] (https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases)) )
* mkdir build && cd build && cmake .. && make
  
  It should work fine on linux and windows/mingw.
  

