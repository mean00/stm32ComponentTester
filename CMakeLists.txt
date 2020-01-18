# Select platform and installation for extra arduino files AND toolchain (compiler...)
SET(PLATFORM_ARCHITECTURE "STM32F1")
# Must be named arduino
#  else the cmake code will look for ARDUINO
# while the name will be the name of the folder
include(./platformConfig.cmake)
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
include_directories(adc)
include_directories(utils)
include_directories(componentLibrary)
include_directories(graphics)
# Build ADC
add_subdirectory(FreeRTOS)
add_subdirectory(adc)
add_subdirectory(graphics)
#
# Our code
#
SET(UTILS utils/fancyLock.cpp    utils/cycleClock.cpp)
SET(COMPT  componentLibrary/Resistor.cpp componentLibrary/Capacitor.cpp componentLibrary/Component.cpp )

ADD_DEFINITIONS(-D__NOP="")
generate_arduino_firmware(stm32ComponentTester 
                SKETCH stm32ComponentTester.ino
                SRCS myCode.cpp testPins.cpp  ${UTILS} ${COMPT} ${GFX}
                PORT ${ARDUINO_DEFAULT_PORT}
                BOARD_CPU ${ARDUINO_CPU}
                )
target_link_libraries(stm32ComponentTester ${libPrefix}ADC)
target_link_libraries(stm32ComponentTester ${libPrefix}FreeRTOS)
target_link_libraries(stm32ComponentTester ${libPrefix}/testerGfx)


# EOF
