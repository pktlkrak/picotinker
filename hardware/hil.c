#include "hil.h"
#include "display/st7789.h"
#include <pico/stdlib.h>
#include <pico/time.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static uint8_t txbuf[240 * 240 * 3];
static volatile int start = 0;

void hil_init() {
    memset(txbuf, 0, sizeof(txbuf));
    display_init();
}

uint8_t *hil_get_display_buffer() { return NULL; }

void hil_display() {
    txbuf[start * 3 + 0] = 0xFF;
    txbuf[start * 3 + 1] = 0xFF;
    txbuf[start * 3 + 2] = 0xFF;

    start += 1;
    if (start == 240 * 240) {
        start = 0;
        memset(txbuf, 0, sizeof(txbuf));
    }

    display_send_framebuffer_dma(txbuf, sizeof(txbuf));
}

hil_button_state_t hil_get_button_state() { return 0; }

void hil_wait() {}

void hil_sleep() {}
