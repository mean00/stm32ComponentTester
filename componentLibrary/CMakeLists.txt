#
SET(WHICH Capacitor_very_low_cap_base.cpp Capacitor_very_low_cap_alt.cpp )


generate_arduino_library(${libPrefix}componentLibrary 
                SRCS Capacitor.cpp  Capacitor_ESR.cpp Component.cpp  Diode.cpp  Resistor.cpp Coil.cpp P_Mosfet.cpp N_Mosfet.cpp Mosfet.cpp NPN_bjt.cpp PNP_bjt.cpp waveForm.cpp Capacitor_low_cap.cpp Capacitor_med_cap.cpp Capacitor_high_cap.cpp ${WHICH}
                BOARD_CPU ${ARDUINO_CPU}
                )
target_include_directories( ${libPrefix}componentLibrary PRIVATE ../adc ../graphics ../control)
# EOF
