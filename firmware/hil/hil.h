#pragma once
#include <stdint.h>

#define SCREEN_SIZE_X 240
#define SCREEN_SIZE_Y 240

#define HIL_BUTTON_UP (1u << 0)
#define HIL_BUTTON_DOWN (1u << 1)
#define HIL_BUTTON_LEFT (1u << 2)
#define HIL_BUTTON_RIGHT (1u << 3)
#define HIL_BUTTON_DPAD_UP (1u << 4)
#define HIL_BUTTON_DPAD_DOWN (1u << 5)
#define HIL_BUTTON_DPAD_LEFT (1u << 6)
#define HIL_BUTTON_DPAD_RIGHT (1u << 7)
#define HIL_BUTTON_START (1u << 8)
#define HIL_BUTTON_SELECT (1u << 9)

typedef uint16_t hil_button_state_t;
typedef uint8_t potentiometer_state_t;

void hil_init();

// *********************** DISPLAY ************************

uint8_t *hil_get_display_buffer();
void hil_display();

// ************************ INPUTS ************************

hil_button_state_t hil_get_button_state();
potentiometer_state_t hil_get_pot1_state();
potentiometer_state_t hil_get_pot2_state();
potentiometer_state_t hil_get_pot3_state();

// ******************** DELAY FUNCTIONS *******************

void hil_wait();
void hil_sleep(int millis);
uint64_t hil_get_time();

// ********************** DEBUGGING ***********************

void hil_debug_init();
void hil_debug_printf(const char *format, ...);
