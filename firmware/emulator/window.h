#pragma once
#include "../hil/hil.h"

void window_init();
void window_render();
void window_poll_events();
void window_wait();
hil_button_state_t window_get_current_button_state();
