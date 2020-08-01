
SET(SRCS dso_menuEngine.cpp)

generate_arduino_library(${libPrefix}menu
                            SRCS ${SRCS}
                            BOARD ${ARDUINO_DEFAULT_BOARD}
                        )
target_include_directories(${libPrefix}menu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Rotary  ../control ../graphics)