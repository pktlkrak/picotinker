#pragma once
#include <stdint.h>

#define HIL_BUTTON_UP (1u << 0)
#define HIL_BUTTON_DOWN (1u << 1)
#define HIL_BUTTON_LEFT (1u << 2)
#define HIL_BUTTON_RIGHT (1u << 3)
#define HIL_BUTTON_DPAD_UP (1u << 4)
#define HIL_BUTTON_DPAD_DOWN (1u << 5)
#define HIL_BUTTON_DPAD_LEFT (1u << 6)
#define HIL_BUTTON_DPAD_RIGHT (1u << 7)

typedef uint8_t hil_button_state_t;

void hil_init();

uint8_t *hil_get_display_buffer();

void hil_display();

hil_button_state_t hil_get_button_state();

void hil_wait();

void hil_sleep();
