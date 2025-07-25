#pragma once

#include <SDL2/SDL.h>

#include "level.h"
#include "sprite_sheet/img.h"
#include "sprite_sheet/sprite_sheet.h"

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

void state_init(void);
void state_destroy(void);

extern state_t state;
