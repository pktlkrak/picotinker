#pragma once

#include <SDL3/SDL_render.h>
#include <stdint.h>

void render_texture_init(SDL_Renderer *renderer);

uint8_t *render_texture_get_buffer();

void render_texture_blit(SDL_Renderer *renderer, int window_size_x,
                         int window_size_y);
