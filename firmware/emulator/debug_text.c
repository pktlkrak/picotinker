#include "debug_text.h"
#include "utils.h"
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

static char window_debug_text_buffer[500];
static uint64_t window_debug_text_time_start = 0;

void window_debug_text_show(const char *format, ...) {
    va_list vl;
    va_start(vl, format);
    vsnprintf(window_debug_text_buffer, sizeof(window_debug_text_buffer),
              format, vl);
    window_debug_text_time_start = SDL_GetTicks();
    va_end(vl);
}

void window_debug_text_render(SDL_Renderer *renderer) {
    if (SDL_GetTicks() - window_debug_text_time_start <
        WINDOW_DEBUG_TEXT_DURATION) {
        // The font of RenderDebugText is 8x8 - the rect has to be a bit bigger,
        // so in both axes I'm adding 2 pixels
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_FRect rect = {WINDOW_DEBUG_TEXT_X - 2, WINDOW_DEBUG_TEXT_Y - 2,
                          8 * strlen(window_debug_text_buffer) + 4, 12};
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderDebugText(renderer, WINDOW_DEBUG_TEXT_X, WINDOW_DEBUG_TEXT_Y,
                            window_debug_text_buffer);
    }
}
