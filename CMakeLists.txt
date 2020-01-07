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
SET(libPrefix ${ARDUINO_DEFAULT_BOARD}_)
#
include_directories(${CMAKE_SOURCE_DIR})
include_directories(FreeRTOS/)
include_directories(FreeRTOS/Source/include)
#  Build ucglib
include(ucg.cmake)
# Build ADC
add_subdirectory(FreeRTOS)
add_subdirectory(adc)
#
# Our code
#
ADD_DEFINITIONS(-D__NOP="")
generate_arduino_firmware(stm32ComponentTester 
                SKETCH stm32ComponentTester.ino
                SRCS myCode.cpp fancyLock.cpp ucg/cppsrc/Ucglib.cpp ${ucg_src} testPins.cpp Resistor.cpp
                PORT ${ARDUINO_DEFAULT_PORT}
                BOARD_CPU ${ARDUINO_CPU}
                )
target_link_libraries(stm32ComponentTester UCG)
target_link_libraries(stm32ComponentTester ${libPrefix}ADC)
target_link_libraries(stm32ComponentTester ${libPrefix}FreeRTOS)


# EOF
