#!/bin/sh
conv()
{
rm -f converted/$1*
convert  $1.png   -monochrome -flatten generated/$1_preview.png
python3 convert.py generated/$1_preview.png generated/$1_compressed.h generated/$1_decl.h $1
}
conv resistor
conv cap
conv splash
