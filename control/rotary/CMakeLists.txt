
SET(SRCS wav_irotary.cpp  Rotary/Rotary.cpp testerControl.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Rotary)
generate_arduino_library(${libPrefix}control
                            SRCS ${SRCS}
                            BOARD ${ARDUINO_DEFAULT_BOARD}
                        )
