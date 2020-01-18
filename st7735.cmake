#SET(ST7735 ${PLATFORM_PATH}/libraries/Adafruit_ILI9341_STM)
SET(ST7735 ${CMAKE_CURRENT_SOURCE_DIR}/ST7735/)
SET(ADA_AS ${CMAKE_CURRENT_SOURCE_DIR}/libraries/Adafruit_GFXS-Library)
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Adafruit-GFX-Library)
include_directories(${ST7735})
include_directories(${ADA_AS})
set(TFT7735_src ${ST7735}/Adafruit_ST7735.cpp)
include_directories(${ST7735})
generate_arduino_library(TFT7735 
                        SRCS ${TFT7735_src} Adafruit-GFX-Library/Adafruit_GFX.cpp  Adafruit-GFX-Library/Adafruit_SPITFT.cpp
                        BOARD_CPU ${ARDUINO_CPU}
                        )


