#include "window.h"

#include "render_texture.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_SIZE_X 720
#define WINDOW_SIZE_Y 720

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

void window_init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        exit(-1);
    }

    if (!SDL_CreateWindowAndRenderer("PicoTinker", WINDOW_SIZE_X, WINDOW_SIZE_Y,
                                     0, &window, &renderer)) {
        fprintf(stderr, "Error creating SDL window: %s\n", SDL_GetError());
        exit(-1);
    }

    render_texture_init(renderer);

    SDL_SetRenderVSync(renderer, 1);
    SDL_ShowWindow(window);
}

void window_render() {
    int window_size_x, window_size_y;
    if (!SDL_GetWindowSizeInPixels(window, &window_size_x, &window_size_y)) {
        fprintf(stderr, "SDL_GetWindowSizeInPixels error: %s\n",
                SDL_GetError());
        exit(-1);
    }

    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderClear(renderer);

    render_texture_blit(renderer, window_size_x, window_size_y);

    if (!SDL_RenderPresent(renderer)) {
        fprintf(stderr, "SDL_RenderPresent error: %s\n", SDL_GetError());
        exit(-1);
    }
}

static void window_handle_events(const SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(0);
    }
}

void window_poll_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        window_handle_events(&event);
    }
}
