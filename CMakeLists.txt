OPTION(USE_GD32F103   "Use GD32F103 MCU" FALSE) 
# Select platform and installation for extra arduino files AND toolchain (compiler...)
SET(PLATFORM_ARCHITECTURE "STM32F1")
# Do we have patch around ?
include(cmake_utils/FindPatch.cmake)
#
include(./platformConfig.cmake)
IF(NOT EXISTS "${PLATFORM_PATH}/patched")
    MESSAGE(STATUS "Patching file in ${PLATFORM_PATH} to add gd32f303/103 support")
    patch_file_p(2 "${PLATFORM_PATH}" "${CMAKE_SOURCE_DIR}/patches/gd32_patch.diff")
    file(WRITE "${PLATFORM_PATH}/patched" "patched")
ENDIF(NOT EXISTS "${PLATFORM_PATH}/patched")

cmake_minimum_required(VERSION 3.0)

IF(USE_GD32F103)
    SET(MCU_SPEED             96000000)                 # Max speed, above 96M USB will not work (?)
ENDIF(USE_GD32F103)
include(./platformConfig.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
SET(CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake)

Project("componentTester" C CXX ASM)

# Board setup
IF(USE_GD32F103)
    SET(EXTENSION             "_GD32F103")
    set(ARDUINO_DEFAULT_BOARD genericGD32F103C)        # Default Board ID, when not specified
    set(ARDUINO_CPU           GD32F103C8)
    ADD_DEFINITIONS("-DHIGH_SPEED_ADC")

ELSE(USE_GD32F103)
    SET(EXTENSION             "_STM32F103")
    set(ARDUINO_DEFAULT_BOARD genericSTM32F103C)        # Default Board ID, when not specified
    set(ARDUINO_CPU           STM32F103C8)
ENDIF(USE_GD32F103)

IF(NOT DEFINED CONFIGURATION)
    SET(CONFIGURATION default)
ENDIF(NOT DEFINED CONFIGURATION)



MESSAGE(STATUS "Starting Component Tester")
ADD_DEFINITIONS("-g3")

#print_board_list()
set(ARDUINO_DEFAULT_PORT ttyACM0) # Default Port, when not specified
set(ARDUINO_CPUMENU .menu.cpu.${ARDUINO_CPU})
SET(libPrefix ${ARDUINO_DEFAULT_BOARD}_)
#
MESSAGE(STATUS "**** Configuration is ** ${CONFIGURATION} **")
include_directories(configurations/${CONFIGURATION})
ADD_DEFINITIONS(-D__NOP="" -DTESTER_CONFIGURATION="${CONFIGURATION}")
SET(SUB_COMPONENTS FreeRTOS adc graphics assets componentLibrary )
FOREACH(comp ${SUB_COMPONENTS} control)
    include_directories(${comp} control)
ENDFOREACH(comp ${SUB_COMPONENTS})

include_directories(${CMAKE_SOURCE_DIR})
include_directories(FreeRTOS/Source/include)
include_directories(utils)
include_directories(componentLibrary)

# Build 
FOREACH(comp ${SUB_COMPONENTS} control)
    add_subdirectory(${comp})
ENDFOREACH(comp ${SUB_COMPONENTS} control)
#
# Our code
#
SET(UTILS utils/fancyLock.cpp    utils/cycleClock.cpp)

include_directories(embedded_printf)
generate_arduino_firmware(componentTester${EXTENSION}
                SKETCH stm32ComponentTester.ino
                SRCS myCode.cpp testPins.cpp  pinAutoTest.cpp ${UTILS}  ${GFX} componentSignature.cpp deltaADC.cpp nvm.cpp calibration.cpp cpuID.cpp embedded_printf/printf.c menuSystem.cpp control/testerControl.cpp helpers/helper_pwm.cpp
                PORT ${ARDUINO_DEFAULT_PORT}
                BOARD_CPU ${ARDUINO_CPU}
                )
FOREACH(comp ${SUB_COMPONENTS})
    target_link_libraries(componentTester${EXTENSION} ${libPrefix}${comp})
ENDFOREACH(comp ${SUB_COMPONENTS})
target_link_libraries(componentTester${EXTENSION} ${libPrefix}RotaryEncoder)
target_link_libraries(componentTester${EXTENSION} ${libPrefix}fonts)
#dump_all()
if(USE_GD32F103)
    MESSAGE(STATUS "\tUsing GD32F103 MCU ")
else(USE_GD32F103)
    MESSAGE(STATUS "\tUsing STM32F103 MCU ")
endif(USE_GD32F103)

# EOF
