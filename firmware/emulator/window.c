#include "window.h"

#include "hil.h"
#include "render_texture.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_SIZE_X 720
#define WINDOW_SIZE_Y 720

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static hil_button_state_t current_button_state = 0;

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

static hil_button_state_t
window_map_sdl_key_to_hil(const SDL_KeyboardEvent *key_event) {
    switch (key_event->key) {
    case SDLK_UP:
        return HIL_BUTTON_DPAD_UP;
    case SDLK_DOWN:
        return HIL_BUTTON_DPAD_DOWN;
    case SDLK_RIGHT:
        return HIL_BUTTON_DPAD_RIGHT;
    case SDLK_LEFT:
        return HIL_BUTTON_DPAD_LEFT;
    case SDLK_1:
        return HIL_BUTTON_START;
    case SDLK_2:
        return HIL_BUTTON_SELECT;
    case SDLK_W:
        return HIL_BUTTON_UP;
    case SDLK_S:
        return HIL_BUTTON_DOWN;
    case SDLK_A:
        return HIL_BUTTON_LEFT;
    case SDLK_D:
        return HIL_BUTTON_RIGHT;
    default:
        return 0;
    }
}

static void window_handle_events(const SDL_Event *event) {
    switch (event->type) {
    case SDL_EVENT_QUIT:
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(0);
    case SDL_EVENT_KEY_DOWN:
        current_button_state |= window_map_sdl_key_to_hil(&event->key);
        break;
    case SDL_EVENT_KEY_UP:
        current_button_state &= ~window_map_sdl_key_to_hil(&event->key);
        break;
    }
}

void window_poll_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        window_handle_events(&event);
    }
}

hil_button_state_t window_get_current_button_state() {
    return current_button_state;
}
