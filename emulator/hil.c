#include "hil.h"

#include "window.h"
#include <stddef.h>

void hil_init() { window_init(); }

uint8_t *hil_get_display_buffer() { return NULL; }

void hil_display() {
    window_poll_events();
    window_render();
}

hil_button_state_t hil_get_button_state() { return 0; }

void hil_wait() {}

void hil_sleep() {}
