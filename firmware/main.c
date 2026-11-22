#include "hil.h"
#include "apps/tetris.h"
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
    hil_button_state_t btns;
    hil_init();
    hil_debug_init();
    hil_debug_printf(
        "Hello, the answer to life, universe and everything is %d\n", 42);

    while (1) {
        btns = hil_get_button_state();
        step(&btns);
        draw();
        hil_display();
    }
}
