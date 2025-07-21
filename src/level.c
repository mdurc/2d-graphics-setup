#include "level.h"

#include "state.h"

static room_t room_template, room1;
static level_t level_template, level1;

void build_levels() {
  room_template = (room_t){.id = 0,
                           .x = 5,
                           .y = 5,
                           .w = 7,
                           .h = 7,
                           .tiles = {{0, 1, 1, 1, 1, 1, 0},
                                     {1, 1, 0, 0, 0, 1, 1},
                                     {1, 0, 1, 0, 1, 0, 1},
                                     {1, 0, 0, 1, 0, 0, 1},
                                     {1, 0, 1, 0, 1, 0, 1},
                                     {1, 1, 0, 0, 0, 1, 1},
                                     {0, 1, 1, 1, 1, 1, 0}},
                           .tile_sheet = {0}};
  level_template = (level_t){.id = 0,
                             .name = "level template",
                             .room_count = 1,
                             .rooms = {&room_template}};

  room1 = (room_t){.id = 0,
                   .x = 12,
                   .y = 5,
                   .w = 7,
                   .h = 7,
                   .tiles = {{0, 1, 1, 1, 1, 1, 0},
                             {1, 1, 0, 0, 0, 1, 1},
                             {1, 0, 1, 0, 1, 0, 1},
                             {1, 0, 0, 1, 0, 0, 1},
                             {1, 0, 1, 0, 1, 0, 1},
                             {1, 1, 0, 0, 0, 1, 1},
                             {0, 1, 1, 1, 1, 1, 0}},
                   .tile_sheet = {0}};
  level1 = (level_t){
      .id = 1, .name = "level one", .room_count = 1, .rooms = {&room1}};
}

void set_level(state_t* state, int n) {
  ASSERT(n < MAX_LEVELS && n <= state->loaded_levels);
  level_t* level = NULL;

  switch (n) {
    case 0: level = &level_template; break;
    case 1: level = &level1; break;
    default: ASSERT(false, "invalid level number"); break;
  }

  if (n < state->loaded_levels) {
    state->current_level = n;
    return;
  }

  for (int i = 0; i < level->room_count; ++i) {
    // bg by default for now
    sprites_init(&level->rooms[i]->tile_sheet, state, "res/bg.png", 8, 8, 4.0f);
  }
  state->current_level = state->loaded_levels;
  state->levels[state->loaded_levels++] = level;
}

void render_level(state_t* state) {
  ASSERT(state->current_level < MAX_LEVELS &&
         state->levels[state->current_level]);
  level_t* level = state->levels[state->current_level];
  for (int i = 0; level && i < level->room_count; ++i) {
    render_batch(state, &level->rooms[i]->tile_sheet, true);
  }
}

void push_level(state_t* state) {
  ASSERT(state && state->current_level < MAX_LEVELS &&
         state->levels[state->current_level]);
  // write out the level name at the bottom left corner (0 indexed)
  int lines_to_fit_font = SCREEN_HEIGHT / (state->font_sheet.scale *
                                           state->font_sheet.sprite_height);
  level_t* level = state->levels[state->current_level];

  ASSERT(level->name, "must build the levels first");
  char level_msg[32];
  snprintf(level_msg, sizeof(level_msg), "%s <id=%d> (rooms=%d)", level->name,
           level->id, level->room_count);

  push_font_str(
      &state->font_sheet, level_msg,
      (fv2){1.0f * state->font_sheet.sprite_width,
            (lines_to_fit_font - 2) * state->font_sheet.sprite_height});

  for (int i = 0; i < level->room_count; ++i) {
    room_t* room = level->rooms[i];
    int sprite_sheet_width =
        room->tile_sheet.img.width / room->tile_sheet.sprite_width;
    for (int r_x = 0; r_x < room->w; ++r_x) {
      for (int r_y = 0; r_y < room->h; ++r_y) {
        int tile_value = room->tiles[r_y][r_x];
        // t = y * w + x
        int src_y = tile_value / sprite_sheet_width;
        int src_x = tile_value % sprite_sheet_width;
        *dynlist_append(room->tile_sheet.batch) = (sprite_t){
            .src_idx = (iv2){src_x, src_y},
            .dst_px = (fv2){(room->x + r_x) * room->tile_sheet.sprite_width,
                            (room->y + r_y) * room->tile_sheet.sprite_height}};
      }
    }
  }
}

void destroy_levels(state_t* state) {
  for (int i = 0; i < state->loaded_levels; ++i) {
    level_t* level = state->levels[i];
    for (int j = 0; level && j < level->room_count; ++j) {
      destroy_sheet(&level->rooms[j]->tile_sheet);
    }
  }
}
