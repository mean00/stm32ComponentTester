#
generate_arduino_library(${libPrefix}componentLibrary 
                SRCS Capacitor.cpp  Capacitor_ESR.cpp Component.cpp  Diode.cpp  Resistor.cpp Coil.cpp P_Mosfet.cpp
                BOARD_CPU ${ARDUINO_CPU}
                )
# EOF
