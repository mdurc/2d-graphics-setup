#ifndef LEVEL_H
#define LEVEL_H

#include "sprites.h"

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

void build_levels(void);
void set_level(state_t* state, int n);
void render_level(state_t* state);
void push_level(state_t* state);

void destroy_levels(state_t* state);

#endif
