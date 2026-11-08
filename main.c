#include "graphics/graphics.h"
#include "hil.h"
#include <string.h>

void super_advanced_fps_meter_draw() {
    static volatile int start = 0;

    uint8_t *buffer = hil_get_display_buffer();

    buffer[start * 3 + 0] = 0xFF;
    buffer[start * 3 + 1] = 0xFF;
    buffer[start * 3 + 2] = 0xFF;

    start += 1;
    if (start == 240 * 240) {
        start = 0;
        memset(buffer, 0, SCREEN_SIZE_X * SCREEN_SIZE_Y * 3);
    }
}

int main() {
    hil_init();

    while (1) {
        super_advanced_fps_meter_draw();
        hil_display();
    }
}
