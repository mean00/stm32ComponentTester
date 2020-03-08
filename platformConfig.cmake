SET(ARDUINO_USE_NEWLIB 1)
SET(PLATFORM_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Arduino_STM32/STM32F1/")
SET(ARDUINO_UPLOAD_METHOD BMP) # Use blackmagic link, no bootloader
MESSAGE(STATUS "HOST SYSTEM ${CMAKE_HOST_SYSTEM_NAME} ")
# Warning the paths are mingw i.e. c:\dev becomes /c/dev
IF("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
    SET(ARDUINO_SDK_PATH "/c/dev/Arduino")
    SET(PLATFORM_TOOLCHAIN_PATH  "/c/dev/arm83/bin")
ELSE()
    # ARDUINO_SDK_PATH should be autodetected
    SET(PLATFORM_TOOLCHAIN_PATH "/home/fx/Arduino_stm32/arm-none-eabi-gcc/download/gcc-arm-none-eabi-8.2.1-1.7/bin")
ENDIF()
