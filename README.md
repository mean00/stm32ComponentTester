# STM32 Component tester


![screenshot](web/demo2.jpg?raw=true "front")

## Scope


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
* Edit platformConfig.cmake to set the path to your compiler (i strongly suggest to use that ( [one](https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases)) )
* mkdir build && cd build && cmake .. && make
  
  It should work fine on linux and windows/mingw.
  
## How does it work
The code is in two parts :
 * Identifying the component
 * Probing the component, using only the MCU ADC in single or dual mode. The can go down to ~ 1us (0.5 in fast dual mode)
 * Since we dont have a DAC on the bluepill, we'll use the charge/discharge of  the parasitic FET capacitor to act as a sort of DAC

  ## Restriction
While accuracy is not bad at all the following restrictions apply :
* Mosfet with VGsOn> 3.3 v will not be probed correctly
* Inductors are disabled, not working properly
* Capacitor of less than 10 pF will not be probed correctly
* Capacitor can be reverse charged with a small current. I'm unsure if that's a big deal. The current is a few mA.


