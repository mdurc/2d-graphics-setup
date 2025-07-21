#ifndef STATE_H
#define STATE_H

#include <SDL2/SDL.h>

#include "c-lib/types.h"
#include "img.h"
#include "level.h"
#include "sprites.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

typedef struct state { // for forward declaration
  SDL_Window* window;
  SDL_Renderer* renderer;
  bool quit;

  sprite_sheet_t font_sheet;
  sprite_sheet_t bg_sheet;

  int current_level, loaded_levels;
  level_t* levels[MAX_LEVELS];
} state_t;

void initialize_state(state_t* state, const char* window_name);

#endif
