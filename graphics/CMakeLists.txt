SET(ADA    ${CMAKE_CURRENT_SOURCE_DIR}/Adafruit-GFX)
SET(ADA_SRC ${ADA}/Adafruit_GFX.cpp  ${ADA}/Adafruit_SPITFT.cpp )
#
# For ST7735
#
# Ovveride the .h file so attributes are protected and not private
include_directories(ST7735 ${CMAKE_CURRENT_SOURCE_DIR}/ST7735/Adafruit-ST7735-mod)
#
SET(ST7735 ${CMAKE_CURRENT_SOURCE_DIR}/ST7735/Adafruit-ST7735)
SET(ST7735_EX ${CMAKE_CURRENT_SOURCE_DIR}/ST7735/)
SET(GFX    ${ST7735}/Adafruit_ST7735.cpp)
SET(TESTER_GFX ${ST7735_EX}/ST7735_Gfx.cpp ${ST7735_EX}/ST7735_ex.cpp ${ST7735_EX}/ST7735_ex_glyph.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../assets)
include_directories(${ST7735})
include_directories(${ADA})
include_directories(${ST7735})

generate_arduino_library(${libPrefix}graphics 
                            SRCS ${GFX} ${ADA_SRC} ${TESTER_GFX}
                            BOARD ${ARDUINO_DEFAULT_BOARD}
                        )
