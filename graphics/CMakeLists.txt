

ADD_SUBDIRECTORY(simplerST7735/platformExample/stm32duino)
generate_arduino_library(${libPrefix}llGfx
                            SRCS ST7735_Gfx.cpp
                            BOARD ${ARDUINO_DEFAULT_BOARD})
target_include_directories(${libPrefix}llGfx PRIVATE simplerST7735/platformExample/stm32duino simplerST7735)