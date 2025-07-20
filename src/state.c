#include "state.h"

#include "c-lib/misc.h"

void initialize_state(state_t* state, const char* window_name) {
  ASSERT(state, "null arguments in initialize_state");

  ASSERT(!SDL_Init(SDL_INIT_VIDEO), "failed SDL_Init: %s\n", SDL_GetError());

  state->window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                                   SCREEN_HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
  ASSERT(state->window, "failed to create sdl window: %s\n", SDL_GetError());

  state->renderer = SDL_CreateRenderer(
      state->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  ASSERT(state->renderer, "failed to create sdl renderer: %s\n",
         SDL_GetError());
}
