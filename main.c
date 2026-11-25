#include "bad_apple.h"
#include "hardware/display/st7789.h"
#include "hil/hil.h"
#include "hil/environment.h"
#include <stdint.h>
#include <stdbool.h>


PICOTINKER_APP("Bad Apple");

uint8_t expanded_frame[7200];

bool XzDecode(const uint8_t *InputBuffer, uint32_t InputSize,
              uint8_t *OutputBuffer, uint32_t *OutputSize);

void bad_apple() {
    const uint8_t *frames_main = FRAMES;
    const uint8_t *frames_end = FRAMES + sizeof(FRAMES);
    uint32_t x;

    while (frames_main < frames_end) {
        x = sizeof(expanded_frame);
        uint8_t *frame = hil_get_display_buffer();
        uint64_t start = hil_get_time_us();
        uint16_t size = ((uint16_t) frames_main[0]) | (((uint16_t) frames_main[1]) << 8);
        frames_main += 2;

        XzDecode(frames_main, size, expanded_frame, &x);
        for (int i = 0; i < SCREEN_SIZE_X * SCREEN_SIZE_Y * 3; i++) {
            frame[i] = ((expanded_frame[i / 24] >> (7 - ((i / 3) & 7))) & 1)
                           ? 0xFF
                           : 0;
        }

        frames_main += size;
        hil_display();
        while ((hil_get_time_us() - start) < 43470) {
            hil_tight_loop();
        }
    }
}

int main() {
    hil_init();

    while (1) {
        bad_apple();
    }
}
