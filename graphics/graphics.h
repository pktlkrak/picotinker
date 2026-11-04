#pragma once

#include "color.h"
#include "hil.h"
#include <stddef.h>
#include <stdint.h>

void graphics_draw_pixel(uint8_t x, uint8_t y, color_t color) {
    if (x > SCREEN_SIZE_X || y > SCREEN_SIZE_Y) {
        return;
    }

    uint8_t *buffer = hil_get_display_buffer();
    size_t addr = y * SCREEN_SIZE_X * 3 + x * 3;
    buffer[addr] = color.r;
    buffer[addr + 1] = color.g;
    buffer[addr + 2] = color.b;
}
