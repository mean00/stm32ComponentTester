include_directories(${CURRENT_CMAKE_SOURCE})
set(TFT_eSPI_src TFT_eSPI/TFT_eSPI.cpp)
generate_arduino_library(TFT_eSPI 
                        SRCS ${TFT_eSPI_src}
                        BOARD_CPU ${ARDUINO_CPU}
                        )

include_directories(TFT_eSPI)

