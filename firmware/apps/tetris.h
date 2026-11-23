#pragma once

#include "hil.h"

void tetris_init(void);
void tetris_step(hil_button_state_t *btn_state);
void tetris_draw(void);