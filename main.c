#include "bad_apple.h"
#include "hardware/display/st7789.h"
#include "hil.h"
#include <pico/time.h>
#include <stdint.h>

uint8_t frame[240 * 240 * 3];
uint8_t expanded_frame[7200];

bool XzDecode(const uint8_t *InputBuffer, uint32_t InputSize,
              uint8_t *OutputBuffer, uint32_t *OutputSize);

void bad_apple() {
    const uint8_t *frames_main = FRAMES;
    const uint8_t *frames_end = FRAMES + sizeof(FRAMES);
    uint32_t x;

    while (frames_main < frames_end) {
        uint64_t start = time_us_64();
        uint16_t size = (frames_main[0]) | (frames_main[1] << 8);
        frames_main += 2;

        XzDecode(frames_main, size, expanded_frame, &x);
        for (int i = 0; i < sizeof(frame); i++) {
            frame[i] = ((expanded_frame[i / 24] >> (7 - ((i / 3) & 7))) & 1)
                           ? 0xFF
                           : 0;
        }

        frames_main += size;
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
