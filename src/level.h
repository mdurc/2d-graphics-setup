#pragma once

#include "sprite_sheet/sprite_sheet.h"

#define MAX_LEVELS 5
#define MAX_ROOMS 5
#define MAX_ROOM_WIDTH  16
#define MAX_ROOM_HEIGHT 16

typedef struct {
  int id;
  int x, y; // top left starting point indices of room
  int w, h;

  // int is the position in the tile_sheet as a 1d array (-1 is a blank space)
  int tiles[MAX_ROOM_HEIGHT][MAX_ROOM_WIDTH];
  sprite_sheet_t tile_sheet;
} room_t;

typedef struct level {
  int id;
  const char* name;
  int room_count;
  room_t* rooms[MAX_ROOMS];
} level_t;

void level_init_all(void);
void level_set_current(int id);
void push_level(sprite_t** batch, level_t* level);
void level_destroy(level_t* level);
