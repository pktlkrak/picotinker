#pragma once

#include "color.h"
#include <stddef.h>
#include <stdint.h>

void graphics_draw_pixel(int16_t x, int16_t y, color_t color);
void graphics_draw_pixel_fast(int16_t x, int16_t y, color_t color);
color_t graphics_get_pixel(int16_t x, int16_t y);
void graphics_draw_bitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                          int16_t w, int16_t h);
