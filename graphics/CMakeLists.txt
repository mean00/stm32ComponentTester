SET(ADA    ${CMAKE_CURRENT_SOURCE_DIR}/Adafruit-GFX-Library)
#
# For ST7735
#
SET(ST7735 ${CMAKE_CURRENT_SOURCE_DIR}/Adafruit-ST7735-Library)
SET(GFX    ${CMAKE_CURRENT_SOURCE_DIR}ST7735/Adafruit-ST7735/ST7735_ex.cpp $ {CMAKE_CURRENT_SOURCE_DIR}ST7735/ST7735_Gfx.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ST7735/stubs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${ST7735})
include_directories(${ADA})
include_directories(${ST7735})

generate_arduino_library(${libPrefix}testerGfx 
                            SRCS ${GFX}
                            BOARD ${ARDUINO_DEFAULT_BOARD}
                        )
