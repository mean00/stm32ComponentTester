# SET(ARDUINO_USE_FLOAT_PRINTF 1) Use this if you use sprintf("%f") => it costs ~ 6 kB
SET(ARDUINO_USE_NEWLIB 1)
SET(PLATFORM_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Arduino_STM32/STM32F1/")
MESSAGE(STATUS "HOST SYSTEM ${CMAKE_HOST_SYSTEM_NAME} ")
# Warning the paths are mingw i.e. c:\dev becomes /c/dev
IF("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    # ARDUINO_SDK_PATH should be autodetected
    SET(PLATFORM_TOOLCHAIN_PATH "/home/fx/Arduino_stm32/arm-none-eabi-gcc/download/gcc-arm-none-eabi-8.2.1-1.7/bin")
#    SET(PLATFORM_TOOLCHAIN_PATH "/usr/local/mcuxpressoide-11.1.1_3241/ide/plugins/com.nxp.mcuxpresso.tools.linux_11.1.0.202001081728/tools/bin/")
ELSE()
    SET(ARDUINO_SDK_PATH "/c/dev/Arduino")
    SET(PLATFORM_TOOLCHAIN_PATH  "/c/dev/arm83/bin")
ENDIF()
