
SET(SRCS src/printf.c)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
generate_arduino_library(${libPrefix}eprintf
                            SRCS ${SRCS}
                            BOARD ${ARDUINO_DEFAULT_BOARD}
                        )
target_compile_definitions(${libPrefix}eprintf PRIVATE PRINTF_DISABLE_SUPPORT_PTRDIFF_T PRINTF_DISABLE_SUPPORT_EXPONENTIAL PRINTF_DISABLE_SUPPORT_LONG_LONG PRINTF_DISABLE_SUPPORT_PTRDIFF_T )

