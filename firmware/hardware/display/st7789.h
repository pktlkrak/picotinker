#pragma once
#include <stdint.h>

#define ST7789_DC_SLEEP_US 10

#define ST7789_SWRESET 0x01
#define ST7789_SLPOUT 0x11
#define ST7789_NORON 0x13
#define ST7789_INVON 0x21
#define ST7789_DISPON 0x29
#define ST7789_CASET 0x2A
#define ST7789_RASET 0x2B
#define ST7789_RAMWR 0x2C
#define ST7789_MADCTL 0x36
#define ST7789_COLMOD 0x3A

void display_init();
void display_send_framebuffer_no_dma(uint8_t *data, int length);
void display_send_framebuffer_dma(uint8_t *data, int length);
