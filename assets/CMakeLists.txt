
SET(SRCS assets.cpp  fonts/fonts.c)

generate_arduino_library(${libPrefix}assets 
                            SRCS ${SRCS}
                            BOARD ${ARDUINO_DEFAULT_BOARD}
                        )

target_include_directories(${libPrefix}assets  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})                        
