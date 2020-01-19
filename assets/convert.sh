#!/bin/sh
conv()
{
rm -f $1.h $1_preview.png
convert  $1.png   -monochrome -flatten generated/$1_preview.png
python3 convert.py generated/$1_preview.png generated/$1_compressed.h $1
}
conv resistor
conv cap
conv splash
