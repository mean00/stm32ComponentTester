#
# Main CmakeList for stm32 based component tester
#
#
# Select platform and installation for extra arduino files AND toolchain (compiler...)
# Do we have patch around ?
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(select_mcu_stm32)
include(FindPatch)
#
include(./platformConfig.cmake)
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

IF(NOT EXISTS "${PLATFORM_PATH}/patched")
    MESSAGE(STATUS "Patching file in ${PLATFORM_PATH} to add gd32f303/103 support")
    patch_file_p(2 "${PLATFORM_PATH}" "${CMAKE_SOURCE_DIR}/patches/gd32_patch.diff")
    file(WRITE "${PLATFORM_PATH}/patched" "patched")
ENDIF(NOT EXISTS "${PLATFORM_PATH}/patched")

IF(NOT EXISTS "${PLATFORM_PATH}/patched2")
    MESSAGE(STATUS "Patching file in ${PLATFORM_PATH} to change timer/pin mapping")
    patch_file_p(2 "${PLATFORM_PATH}" "${CMAKE_SOURCE_DIR}/patches/remap_pin_timer2.diff")
    file(WRITE "${PLATFORM_PATH}/patched2" "patched")
ENDIF(NOT EXISTS "${PLATFORM_PATH}/patched2")

IF(NOT EXISTS "${PLATFORM_PATH}/patched3")
    MESSAGE(STATUS "Patching file in ${PLATFORM_PATH} to fix spi dma in async mode")
    patch_file_p(2 "${PLATFORM_PATH}" "${CMAKE_SOURCE_DIR}/patches/spi_dma_async_patch2.diff")
    file(WRITE "${PLATFORM_PATH}/patched3" "patched3")
ENDIF(NOT EXISTS "${PLATFORM_PATH}/patched3")


cmake_minimum_required(VERSION 3.0)

# Set config depending on the MCU selected

SET(CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake)
Project("componentTester" C CXX ASM)

set(ARDUINO_DEFAULT_BOARD ${X_ARDUINO_DEFAULT_BOARD})        # Default Board ID, when not specified
set(ARDUINO_CPU           ${X_ARDUINO_CPU})
SET(ARDUINO_UPLOAD_METHOD ${X_ARDUINO_UPLOAD_METHOD}) # Use blackmagic link, if you comment it out you'll use DFU => 8kB flash
set(ARDUINO_DEFAULT_PORT ttyACM0) # Default Port, when not specified

SET(libPrefix ${ARDUINO_DEFAULT_BOARD}_)

IF(NOT DEFINED CONFIGURATION)
    SET(CONFIGURATION default)
ENDIF(NOT DEFINED CONFIGURATION)


MESSAGE(STATUS "Starting Component Tester")
ADD_DEFINITIONS("-g3")  

#print_board_list()
#
MESSAGE(STATUS "**** Configuration is ** ${CONFIGURATION} **")
include_directories(configurations/${CONFIGURATION})
ADD_DEFINITIONS(-D__NOP="" -DTESTER_CONFIGURATION="${CONFIGURATION}")
SET(SUB_COMPONENTS FreeRTOS adc  assets componentLibrary menu eprintf)


include_directories(eprintf/src)
include_directories(${CMAKE_SOURCE_DIR})
include_directories(FreeRTOS/Source/include)
include_directories(FreeRTOS/)
include_directories(utils)
include_directories(adc)
include_directories(graphics)
include_directories(control)

# Build 
FOREACH(comp ${SUB_COMPONENTS} control graphics)
    add_subdirectory(${comp})
ENDFOREACH(comp ${SUB_COMPONENTS} control)

#
# Our code
#
SET(UTILS   utils/cycleClock.cpp  utils/i2c_stubs.c utils/usb_stubs.c)

generate_arduino_firmware(componentTester${EXTENSION}
                SKETCH stm32ComponentTester.ino
                SRCS myCode.cpp testPins.cpp testPins_pulse.cpp pinAutoTest.cpp ${UTILS}  ${GFX} componentSignature.cpp deltaADC.cpp nvm.cpp calibration.cpp cpuID.cpp  menuSystem.cpp control/testerControl.cpp  tester.cpp dso_logger.cpp
                PORT ${ARDUINO_DEFAULT_PORT}
                BOARD_CPU ${ARDUINO_CPU}
                )
FOREACH(comp ${SUB_COMPONENTS})
    target_link_libraries(componentTester${EXTENSION} ${libPrefix}${comp})
ENDFOREACH(comp ${SUB_COMPONENTS})

target_include_directories(componentTester${EXTENSION} PRIVATE componentLibrary adc  control menu) # graphics

target_link_libraries(componentTester${EXTENSION} ${libPrefix}RotaryEncoder ${libPrefix}llGfx ${libPrefix}simplerST7735)

MESSAGE(STATUS "\tUsing ${EXTENSION} MCU ")

# EOF
