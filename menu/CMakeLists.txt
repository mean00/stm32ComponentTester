
SET(SRCS dso_menuEngine.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Rotary)
generate_arduino_library(${libPrefix}menu
                            SRCS ${SRCS}
                            BOARD ${ARDUINO_DEFAULT_BOARD}
                        )
