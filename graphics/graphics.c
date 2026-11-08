#include "graphics.h"

#include "hil.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

void graphics_draw_pixel(int16_t x, int16_t y, color_t color) {
    if (x < 0 || y < 0 || x >= SCREEN_SIZE_X || y >= SCREEN_SIZE_Y)
        return;
    graphics_draw_pixel_fast(x, y, color);
}

void graphics_draw_pixel_fast(int16_t x, int16_t y, color_t color) {
    uint8_t *buffer = hil_get_display_buffer();
    size_t buffer_pos = (y * SCREEN_SIZE_X + x) * 3;
    buffer[buffer_pos] = color.r;
    buffer[buffer_pos + 1] = color.g;
    buffer[buffer_pos + 2] = color.b;
}

color_t graphics_get_pixel(int16_t x, int16_t y) {
    uint8_t *buffer = hil_get_display_buffer();
    size_t buffer_pos = (y * SCREEN_SIZE_X + x) * 3;
    color_t color = {.r = buffer[buffer_pos],
                     .g = buffer[buffer_pos + 1],
                     .b = buffer[buffer_pos + 2]};
    return color;
}

void graphics_draw_bitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                          int16_t w, int16_t h) {
    uint8_t *buffer = hil_get_display_buffer();
    for (int16_t bitmap_y = 0; bitmap_y < h; bitmap_y++) {
        size_t bitmap_pos = bitmap_y * w * 3;
        size_t screen_pos = ((y + bitmap_y) * SCREEN_SIZE_X + x) * 3;
        memcpy(buffer + screen_pos, bitmap + bitmap_pos, w * 3);
    }
}
