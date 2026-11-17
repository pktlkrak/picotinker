#pragma once
#include <SDL3/SDL_render.h>

#define WINDOW_DEBUG_TEXT_DURATION 500
#define WINDOW_DEBUG_TEXT_X 20
#define WINDOW_DEBUG_TEXT_Y 20

void window_debug_text_render(SDL_Renderer *renderer);
void window_debug_text_show(const char *format, ...);
