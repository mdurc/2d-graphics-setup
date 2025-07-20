#ifndef STATE_H
#define STATE_H

#include <SDL2/SDL.h>

#include "c-lib/types.h"
#include "img.h"
#include "sprites.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

typedef struct state {
  SDL_Window* window;
  SDL_Renderer* renderer;
  sprite_sheet_t font_sheet;
  bool quit;
} state_t;

void initialize_state(state_t* state, const char* window_name);

#endif
