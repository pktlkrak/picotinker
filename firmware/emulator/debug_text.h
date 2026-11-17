#pragma once
#include <SDL3/SDL_render.h>

#define WINDOW_DEBUG_TEXT_DURATION 500
#define WINDOW_DEBUG_TEXT_X 20
#define WINDOW_DEBUG_TEXT_Y 20

void render_debug_text(SDL_Renderer *renderer);
void window_show_debug_text(const char *format, ...);
