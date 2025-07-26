#pragma once

#include <SDL2/SDL.h>

#include "config/config.h"
#include "sprite_sheet/img.h"
#include "sprite_sheet/sprite_sheet.h"
#include "time.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

typedef struct state { // for forward declaration
  SDL_Window* window;
  SDL_Renderer* renderer;

  sprite_sheet_t font_sheet;
  sprite_sheet_t bg_sheet;

  config_t config;
  input_state_t input;
  time_state_t time;
} state_t;

void state_init(void);
void state_destroy(void);

extern state_t state;
