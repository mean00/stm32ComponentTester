# Select platform and installation for extra arduino files AND toolchain (compiler...)
SET(PLATFORM_ARCHITECTURE "STM32F1")
# Must be named arduino
#  else the cmake code will look for ARDUINO
# while the name will be the name of the folder
IF(NOT DEFINED CONFIGURATION)
    SET(CONFIGURATION default)
ENDIF(NOT DEFINED CONFIGURATION)

include(./platformConfig.cmake)
SET(CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake)
MESSAGE(STATUS "Starting Component Tester")
ADD_DEFINITIONS("-g3")
cmake_minimum_required(VERSION 2.8)
Project("stm32ComponentTester" C CXX ASM)

#print_board_list()
set(ARDUINO_DEFAULT_BOARD genericSTM32F103C)        # Default Board ID, when not specified
set(ARDUINO_DEFAULT_PORT ttyACM0) # Default Port, when not specified
set(ARDUINO_CPU          STM32F103C8)
SET(libPrefix ${ARDUINO_DEFAULT_BOARD}_)
#
MESSAGE(STATUS "**** Configuration is ** ${CONFIGURATION} **")
include_directories(configurations/${CONFIGURATION})
ADD_DEFINITIONS(-D__NOP="" -DTESTER_CONFIGURATION="${CONFIGURATION}")
SET(SUB_COMPONENTS FreeRTOS adc graphics assets componentLibrary control)
FOREACH(comp ${SUB_COMPONENTS})
    include_directories(${comp})
ENDFOREACH(comp ${SUB_COMPONENTS})

include_directories(${CMAKE_SOURCE_DIR})
include_directories(FreeRTOS/Source/include)
include_directories(utils)
include_directories(componentLibrary)

# Build 
FOREACH(comp ${SUB_COMPONENTS})
    add_subdirectory(${comp})
ENDFOREACH(comp ${SUB_COMPONENTS})
#
# Our code
#
SET(UTILS utils/fancyLock.cpp    utils/cycleClock.cpp)

include_directories(embedded_printf)
generate_arduino_firmware(stm32ComponentTester 
                SKETCH stm32ComponentTester.ino
                SRCS myCode.cpp testPins.cpp  pinAutoTest.cpp ${UTILS}  ${GFX} componentSignature.cpp deltaADC.cpp nvm.cpp calibration.cpp cpuID.cpp embedded_printf/printf.c menuSystem.cpp control/testerControl.cpp
                PORT ${ARDUINO_DEFAULT_PORT}
                BOARD_CPU ${ARDUINO_CPU}
                )
FOREACH(comp ${SUB_COMPONENTS})
    target_link_libraries(stm32ComponentTester ${libPrefix}${comp})
ENDFOREACH(comp ${SUB_COMPONENTS})
target_link_libraries(stm32ComponentTester ${libPrefix}fonts)
#dump_all()
# EOF
