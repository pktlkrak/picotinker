#include "bad_apple.h"
#include "hardware/display/st7789.h"
#include "hil.h"
#include <pico/time.h>

uint16_t *decompress_bad_apple_frame(uint8_t *output, uint16_t *frames) {
    int x = 0, y = 0;
    int pixels = 0;

    while (pixels < 120 * 120) {
        uint16_t part = *frames++;

        if (part & 0x8000) {
            part &= ~0x8000;
            for (int i = 0; i < part; i++) {
                output[(y * 240 + x) * 3 + 0] = 0xFF;
                output[(y * 240 + x) * 3 + 1] = 0xFF;
                output[(y * 240 + x) * 3 + 2] = 0xFF;

                output[(y * 240 + x + 1) * 3 + 0] = 0xFF;
                output[(y * 240 + x + 1) * 3 + 1] = 0xFF;
                output[(y * 240 + x + 1) * 3 + 2] = 0xFF;

                output[((y + 1) * 240 + x) * 3 + 0] = 0xFF;
                output[((y + 1) * 240 + x) * 3 + 1] = 0xFF;
                output[((y + 1) * 240 + x) * 3 + 2] = 0xFF;

                output[((y + 1) * 240 + x + 1) * 3 + 0] = 0xFF;
                output[((y + 1) * 240 + x + 1) * 3 + 1] = 0xFF;
                output[((y + 1) * 240 + x + 1) * 3 + 2] = 0xFF;
                x += 2;
                if (x == 240) {
                    y += 2;
                    x = 0;
                }
            }
        } else {
            for (int i = 0; i < part; i++) {
                output[(y * 240 + x) * 3 + 0] = 0x00;
                output[(y * 240 + x) * 3 + 1] = 0x00;
                output[(y * 240 + x) * 3 + 2] = 0x00;

                output[(y * 240 + x + 1) * 3 + 0] = 0x00;
                output[(y * 240 + x + 1) * 3 + 1] = 0x00;
                output[(y * 240 + x + 1) * 3 + 2] = 0x00;

                output[((y + 1) * 240 + x) * 3 + 0] = 0x00;
                output[((y + 1) * 240 + x) * 3 + 1] = 0x00;
                output[((y + 1) * 240 + x) * 3 + 2] = 0x00;

                output[((y + 1) * 240 + x + 1) * 3 + 0] = 0x00;
                output[((y + 1) * 240 + x + 1) * 3 + 1] = 0x00;
                output[((y + 1) * 240 + x + 1) * 3 + 2] = 0x00;
                x += 2;
                if (x == 240) {
                    y += 2;
                    x = 0;
                }
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
        uint64_t start = time_us_64();
        frames_main = decompress_bad_apple_frame(frame, frames_main);
        display_send_framebuffer_dma(frame, sizeof(frame));
        while ((time_us_64() - start) < 43470) {
        }
    }
}

int main() {
    hil_init();

    while (1) {
        bad_apple();
    }
}
