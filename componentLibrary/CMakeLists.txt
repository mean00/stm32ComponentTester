#
generate_arduino_library(${libPrefix}componentLibrary 
                SRCS Capacitor.cpp  Capacitor_ESR.cpp Component.cpp  Diode.cpp  Resistor.cpp Coil.cpp P_Mosfet.cpp N_Mosfet.cpp Mosfet.cpp
                BOARD_CPU ${ARDUINO_CPU}
                )
# EOF
