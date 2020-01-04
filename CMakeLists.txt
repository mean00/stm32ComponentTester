# Select platform and installation for extra arduino files AND toolchain (compiler...)
SET(PLATFORM_ARCHITECTURE "STM32F1")
# Must be named arduino
#  else the cmake code will look for ARDUINO
# while the name will be the name of the folder
 SET(PLATFORM_PATH "/home/fx/Arduino/hardware/stm32/STM32F1/")
SET(ARDUINO_USE_NEWLIB 1)
SET(PLATFORM_TOOLCHAIN_PATH "/home/fx/Arduino_stm32/arm-none-eabi-gcc/download/gcc-arm-none-eabi-8.2.1-1.7/bin")

SET(CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake)
MESSAGE(STATUS "Starting hello-stm32")
ADD_DEFINITIONS("-g3")
cmake_minimum_required(VERSION 2.8)
Project("stm32ComponentTester" C CXX ASM)

#print_board_list()


set(ARDUINO_DEFAULT_BOARD genericSTM32F103C)        # Default Board ID, when not specified
set(ARDUINO_DEFAULT_PORT ttyACM0) # Default Port, when not specified
set(ARDUINO_CPU          STM32F103C8)
set(ucg_src
ucg/csrc/ucg_bitmap.c
ucg/csrc/ucg_box.c
ucg/csrc/ucg_ccs.c
ucg/csrc/ucg_circle.c
ucg/csrc/ucg_clip.c
ucg/csrc/ucg_com_msg_api.c
ucg/csrc/ucg_dev_default_cb.c
ucg/csrc/ucg_dev_ic_hx8352c.c
ucg/csrc/ucg_dev_ic_ili9163.c
ucg/csrc/ucg_dev_ic_ili9325.c
ucg/csrc/ucg_dev_ic_ili9325_spi.c
ucg/csrc/ucg_dev_ic_ili9341.c
ucg/csrc/ucg_dev_ic_ili9486.c
ucg/csrc/ucg_dev_ic_ld50t6160.c
ucg/csrc/ucg_dev_ic_pcf8833.c
ucg/csrc/ucg_dev_ic_seps225.c
ucg/csrc/ucg_dev_ic_ssd1289.c
ucg/csrc/ucg_dev_ic_ssd1331.c
ucg/csrc/ucg_dev_ic_ssd1351.c
ucg/csrc/ucg_dev_ic_st7735.c
ucg/csrc/ucg_dev_msg_api.c
ucg/csrc/ucg_dev_oled_128x128_ft.c
ucg/csrc/ucg_dev_oled_128x128_ilsoft.c
ucg/csrc/ucg_dev_oled_128x128_univision.c
ucg/csrc/ucg_dev_oled_160x128_samsung.c
ucg/csrc/ucg_dev_oled_96x64_univision.c
ucg/csrc/ucg_dev_tft_128x128_ili9163.c
ucg/csrc/ucg_dev_tft_128x160_st7735.c
ucg/csrc/ucg_dev_tft_132x132_pcf8833.c
ucg/csrc/ucg_dev_tft_240x320_ili9325_spi.c
ucg/csrc/ucg_dev_tft_240x320_ili9341.c
ucg/csrc/ucg_dev_tft_240x320_itdb02.c
ucg/csrc/ucg_dev_tft_240x320_ssd1289.c
ucg/csrc/ucg_dev_tft_240x400_hx8352c.c
ucg/csrc/ucg_dev_tft_320x480_ili9486.c
ucg/csrc/ucg_font.c
ucg/csrc/ucg_init.c
ucg/csrc/ucg_line.c
ucg/csrc/ucg_pixel.c
ucg/csrc/ucg_pixel_font_data.c
ucg/csrc/ucg_polygon.c
ucg/csrc/ucg_rotate.c
ucg/csrc/ucg_scale.c
ucg/csrc/ucg_vector_font_data.c)

# this is needed for the nano, it should be set automatically, maybe something changed with sdk 1.8.x ?
set(ARDUINO_CPUMENU .menu.cpu.${ARDUINO_CPU})
#PRINT_SETTINGS(${ARDUINO_DEFAULT_BOARD})

# Where you store your downloaded libs or manually installed
SET(CUSTOM_LIB_DIR /home/fx/Arduino/libraries)
link_directories(${CUSTOM_LIB_DIR}/ucg)
set(ucg_RECURSE True)

link_directories( ${CUSTOM_LIB_DIR})
#
# Be careful with the includes in the ino files
# #include <foo.h> // this is a comment
# will fail
# same with extra tabs or space after the #include "blah.h"
#
SET(libPrefix ${ARDUINO_DEFAULT_BOARD}_)
# This reduces size by 10% or so
#ADD_DEFINITIONS("-ffunction-sections -fdata-sections ")
include_directories(ucg/cppsrc/)
include_directories(ucg/csrc)
ADD_DEFINITIONS(-D__NOP="")
generate_arduino_firmware(stm32ComponentTester 
                SKETCH stm32ComponentTester.ino
                SRCS myCode.cpp fancyLock.cpp ucg/cppsrc/Ucglib.cpp ${ucg_src}
                PORT ${ARDUINO_DEFAULT_PORT}
                BOARD_CPU ${ARDUINO_CPU}
                )

# EOF
