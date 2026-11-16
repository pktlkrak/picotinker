#include "render_texture.h"

#include "hil.h"
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static SDL_Texture *texture = NULL;

static uint8_t render_buffer[SCREEN_SIZE_X * SCREEN_SIZE_Y * 3];

void render_texture_init(SDL_Renderer *renderer) {
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
                                SDL_TEXTUREACCESS_STREAMING, SCREEN_SIZE_X,
                                SCREEN_SIZE_Y);
    if (!texture) {
        fprintf(stderr, "SDL_CreateTexture error: %s\n", SDL_GetError());
        exit(-1);
    }
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
}

uint8_t *render_texture_get_buffer() { return render_buffer; }

static void render_texture_upload() {
    void *locked_texture = NULL;
    int pitch = 0;
    if (!SDL_LockTexture(texture, NULL, &locked_texture, &pitch)) {
        fprintf(stderr, "SDL_LockTexture error: %s\n", SDL_GetError());
        exit(-1);
    }

    for (int row = 0; row < SCREEN_SIZE_Y; row++) {
        memcpy(locked_texture, render_buffer + (row * SCREEN_SIZE_X * 3),
               SCREEN_SIZE_X * 3 * sizeof(uint8_t));
        locked_texture += pitch;
    }

    SDL_UnlockTexture(texture);
}

void render_texture_blit(SDL_Renderer *renderer, int window_size_x,
                         int window_size_y) {
    render_texture_upload();

    SDL_FRect screen_rect = {
        .x = 0, .y = 0, .w = window_size_x, .h = window_size_y};
    if (!SDL_RenderTexture(renderer, texture, NULL, &screen_rect)) {
        fprintf(stderr, "SDL_RenderTexture error: %s\n", SDL_GetError());
        exit(-1);
    }
}
