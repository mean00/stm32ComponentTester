generate_arduino_library(${libPrefix}fonts 
                            SRCS fonts.c
                            BOARD ${ARDUINO_DEFAULT_BOARD}
                        )
target_include_directories(${libPrefix}fonts PRIVATE ../../assets)
