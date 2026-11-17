#include "hil.h"

#include "render_texture.h"
#include "window.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

void hil_init() { window_init(); }

uint8_t *hil_get_display_buffer() { return render_texture_get_buffer(); }

void hil_debug_init() {}
void hil_debug_printf(const char *format, ...) {
    va_list vl;
    va_start(vl, format);
    printf("[HIL DEBUG]: ");
    vprintf(format, vl);
    va_end(vl);
}

void hil_display() {
    window_poll_events();
    window_render();
}

hil_button_state_t hil_get_button_state() { return 0; }

void hil_wait() {}

void hil_sleep(int millis) {}
