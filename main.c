#include "bad_apple.h"
#include "hardware/display/st7789.h"
#include "hil.h"
#include <pico/time.h>

uint16_t *decompress_bad_apple_frame(uint8_t *output, uint16_t *frames) {
    int pixels = 0;

    while (pixels < 240 * 240) {
        uint16_t part = *frames++;

        if (part & 0x8000) {
            part &= ~0x8000;
            for (int i = 0; i < part * 3; i++) {
                *output++ = 0xFF;
            }
        } else {
            for (int i = 0; i < part * 3; i++) {
                *output++ = 0x00;
            }
        }
        pixels += part;
    }

    return frames;
}

uint8_t frame[240 * 240 * 3];

void bad_apple() {
    uint16_t *frames_main = (uint16_t *)FRAMES;
    uint16_t *frames_end = (uint16_t *)(FRAMES + sizeof(FRAMES));

    while (frames_main < frames_end) {
        frames_main = decompress_bad_apple_frame(frame, frames_main);
        display_send_framebuffer_dma(frame, sizeof(frame));
        uint64_t start = time_us_64();
        while (time_us_64() - start < 125000) {
        }
    }
}

int main() {
    hil_init();

    while (1) {
        bad_apple();
    }
}
