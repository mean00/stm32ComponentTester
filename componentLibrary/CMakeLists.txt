#
generate_arduino_library(${libPrefix}componentLibrary 
                SRCS Capacitor.cpp  Component.cpp  Diode.cpp  Resistor.cpp
                BOARD_CPU ${ARDUINO_CPU}
                )
# EOF
