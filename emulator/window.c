#include "window.h"

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
        return;
    }

    if (!SDL_CreateWindowAndRenderer("PicoTinker", WINDOW_SIZE_X, WINDOW_SIZE_Y,
                                     0, &window, &renderer)) {
        fprintf(stderr, "Error creating SDL window: %s\n", SDL_GetError());
        return;
    }

    SDL_ShowWindow(window);
}

void window_render() {
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);
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
