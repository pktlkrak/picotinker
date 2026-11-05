#include "hil.h"

#include "display/st7789.h"
#include <pico/stdlib.h>
#include <pico/time.h>
#include <stdint.h>

static uint8_t render_buffer[2][SCREEN_SIZE_X * SCREEN_SIZE_Y * 3];
static size_t render_buffer_index = 0;

void hil_init() { display_init(); }

uint8_t *hil_get_display_buffer() { return render_buffer[render_buffer_index]; }

void hil_display() {
    display_send_framebuffer_dma(render_buffer[render_buffer_index],
                                 sizeof(render_buffer[render_buffer_index]));
    render_buffer_index = !render_buffer_index;
}

hil_button_state_t hil_get_button_state() { return 0; }

void hil_wait() {}

void hil_sleep() {}
