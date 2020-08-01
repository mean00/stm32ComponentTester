SET(ST7735_EX ${CMAKE_CURRENT_SOURCE_DIR}/ST7735/)
ADD_DEFINITIONS(-DTINY_7735)
SET(ADA_SRC tinyAdafruit_ST7735/tinyAdafruit_ST7735.cpp)
SET(TESTER_GFX ${ST7735_EX}/ST7735_Gfx.cpp ${ST7735_EX}/ST7735_ex.cpp ${ST7735_EX}/ST7735_ex_glyph.cpp)

# Put the fonts in their own library to track the size they consume easily
add_subdirectory(fonts)
# This is because there is spi.h and SPI.h
# it causes problems in case insensitive filesystems
 add_definitions("-include ../../SPI/src/SPI.h")
generate_arduino_library(${libPrefix}graphics 
                            SRCS ${GFX} ${ADA_SRC} ${TESTER_GFX}
                            BOARD ${ARDUINO_DEFAULT_BOARD}
                        )

TARGET_INCLUDE_DIRECTORIES(${libPrefix}graphics  PRIVATE tinyAdafruit_ST7735)                           
                            
                            
