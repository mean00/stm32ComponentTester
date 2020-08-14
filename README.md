# STM32 Component tester


![screenshot](web/demo2.jpg?raw=true "front")
NB: This prototype is  done using a discrete chip, you can also use a bluepill board.
## Scope

This  project is a redone from scratch component tester 
( see https://www.mikrocontroller.net/articles/AVR-Transistortester ).

The tester is powered by a STM32F103/GD32F103 or GD32F303 MCU.
You can also use directly a bluepill board.

The shopping list is (when using a bluepill board)
  * Bluepill board
  * 1.44' SPI 128x128 screen ( [Ebay Link](https://www.ebay.fr/itm/2PCS-1-44-Red-Serial-128X128-SPI-Color-TFT-LCD-Module-Replace-Nokia-5110-LCD/400766556571?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649 ))
  * LMV324i Quad op Amp (or any rail to rail quad op amp)
  * AMS 3.3v voltage regulator regulator
  * (470 Ohms, 15kOhm, 470kOhm) x3
  
If you want to use the MCU directly, look at  kicad/schematic.pdf
Due to the low voltage (3.3v), some mosfet will not be tested correctly.

## Features
 * Resistor
 * Capacitors (from ~ 2 pf to ~ 600 uF)
 * NPN and PNP transistors : Hfe, Base-Emitter forward diode voltage
 * N & P Mosfet : GS capacitance, RDSon, diode voltage, VgsOn

## How to build

* Install arduino and cmake
* Checkout the project, make sure you checkout the submodules
* Edit platformConfig.cmake to set the path to your compiler (i strongly suggest to use that ( [one](https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases)) )
* mkdir build && cd build && cmake .. && make
  
  It should work fine on linux and windows/mingw.
  
## Options
   By default the firmware is generated for STM32F103C8T6
   For the GD32F103 : add -DUSE_GD32F103=True to the cmake command
   For the GD32F303 : add -DUSE_GD32F303=True to the cmake command

## How does it work ?
The code is in two parts :
 * Identifying the component
 * Probing the component, using only the MCU ADC in single or dual mode. 
 * For mosfets, since we dont have a DAC on the bluepill, we'll use the charge/discharge of  the parasitic FET capacitor to act as a sort of DAC

  ## Restriction
While accuracy is not bad at all the following restrictions apply :
* Mosfet with VGsOn> 3.3 v will not be probed correctly
* Inductors are disabled, not working properly
* Capacitor of less than 4 pF will not be probed correctly
* Capacitor can be reverse charged with a small current. I'm unsure if that's a big deal. The current is a few mA.


