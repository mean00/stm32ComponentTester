
set(ucg_src
ucg/csrc/ucg_bitmap.c
ucg/csrc/ucg_box.c
ucg/csrc/ucg_ccs.c
ucg/csrc/ucg_circle.c
ucg/csrc/ucg_clip.c
ucg/csrc/ucg_com_msg_api.c
ucg/csrc/ucg_dev_default_cb.c
ucg/csrc/ucg_dev_ic_hx8352c.c
ucg/csrc/ucg_dev_ic_ili9163.c
ucg/csrc/ucg_dev_ic_ili9325.c
ucg/csrc/ucg_dev_ic_ili9325_spi.c
ucg/csrc/ucg_dev_ic_ili9341.c
ucg/csrc/ucg_dev_ic_ili9486.c
ucg/csrc/ucg_dev_ic_ld50t6160.c
ucg/csrc/ucg_dev_ic_pcf8833.c
ucg/csrc/ucg_dev_ic_seps225.c
ucg/csrc/ucg_dev_ic_ssd1289.c
ucg/csrc/ucg_dev_ic_ssd1331.c
ucg/csrc/ucg_dev_ic_ssd1351.c
ucg/csrc/ucg_dev_ic_st7735.c
ucg/csrc/ucg_dev_msg_api.c
ucg/csrc/ucg_dev_oled_128x128_ft.c
ucg/csrc/ucg_dev_oled_128x128_ilsoft.c
ucg/csrc/ucg_dev_oled_128x128_univision.c
ucg/csrc/ucg_dev_oled_160x128_samsung.c
ucg/csrc/ucg_dev_oled_96x64_univision.c
ucg/csrc/ucg_dev_tft_128x128_ili9163.c
ucg/csrc/ucg_dev_tft_128x160_st7735.c
ucg/csrc/ucg_dev_tft_132x132_pcf8833.c
ucg/csrc/ucg_dev_tft_240x320_ili9325_spi.c
ucg/csrc/ucg_dev_tft_240x320_ili9341.c
ucg/csrc/ucg_dev_tft_240x320_itdb02.c
ucg/csrc/ucg_dev_tft_240x320_ssd1289.c
ucg/csrc/ucg_dev_tft_240x400_hx8352c.c
ucg/csrc/ucg_dev_tft_320x480_ili9486.c
ucg/csrc/ucg_font.c
ucg/csrc/ucg_init.c
ucg/csrc/ucg_line.c
ucg/csrc/ucg_pixel.c
ucg/csrc/ucg_pixel_font_data.c
ucg/csrc/ucg_polygon.c
ucg/csrc/ucg_rotate.c
ucg/csrc/ucg_scale.c
ucg/csrc/ucg_vector_font_data.c)

generate_arduino_library(UCG 
                        SRCS ${ucg_src}
                        BOARD_CPU ${ARDUINO_CPU}
                        )

include_directories(ucg/cppsrc/)
include_directories(ucg/csrc)

