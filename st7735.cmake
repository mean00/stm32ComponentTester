SET(ST7735 ${CMAKE_CURRENT_SOURCE_DIR}/ST7735/)
SET(ADA    ${CMAKE_CURRENT_SOURCE_DIR}/Adafruit-GFX-Library)
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/stubs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${ST7735})
include_directories(${ADA})
include_directories(${ST7735})
#
set(TFT7735_src ${ST7735}/Adafruit_ST7735.cpp)
#
set(ADA_SRC  ${ADA}/Adafruit_GFX.cpp  ${ADA}/Adafruit_SPITFT.cpp)
#
generate_arduino_library(TFT7735 
                        SRCS ${TFT7735_src} ${ADA}/Adafruit_GFX.cpp  ${ADA}/Adafruit_SPITFT.cpp
                        BOARD_CPU ${ARDUINO_CPU}
                        )


