#include "level.h"

#include "c-lib/dynlist.h"
#include "c-lib/misc.h"
#include "font.h"
#include "state.h"

static room_t room_template, room1;
static level_t level_template, level1;

void level_init_all(void) {
  room_template = (room_t){.id = 0,
                           .x = 5,
                           .y = 5,
                           .w = 7,
                           .h = 7,
                           .tiles = {{0, 32, 32, 32, 32, 32, 0},
                                     {32, 32, 0, 0, 0, 32, 32},
                                     {32, 0, 32, 0, 32, 0, 32},
                                     {32, 0, 0, 32, 0, 0, 32},
                                     {32, 0, 32, 0, 32, 0, 32},
                                     {32, 32, 0, 0, 0, 32, 32},
                                     {0, 32, 32, 32, 32, 32, 0}},
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
                   .tiles = {{0, 32, 32, 32, 32, 32, 0},
                             {32, 32, 0, 0, 0, 32, 32},
                             {32, 0, 32, 0, 32, 0, 32},
                             {32, 0, 0, 32, 0, 0, 32},
                             {32, 0, 32, 0, 32, 0, 32},
                             {32, 32, 0, 0, 0, 32, 32},
                             {0, 32, 32, 32, 32, 32, 0}},
                   .tile_sheet = {0}};
  level1 = (level_t){
      .id = 1, .name = "level one", .room_count = 1, .rooms = {&room1}};
}

void level_set_current(int id) {
  ASSERT(id < MAX_LEVELS && id <= state.loaded_levels);
  level_t* level = NULL;

  switch (id) {
    case 0: level = &level_template; break;
    case 1: level = &level1; break;
    default: ERROR_EXIT("invalid level number"); break;
  }

  if (id < state.loaded_levels) {
    // no need to re-allocate the sprite sheet for that level
    state.current_level = id;
    return;
  }

  for (int i = 0; i < level->room_count; ++i) {
    // bg by default for now
    sprite_sheet_init(&level->rooms[i]->tile_sheet, "res/bg.png", 8, 8, 4.0f);
  }
  state.current_level = state.loaded_levels;
  state.levels[state.loaded_levels++] = level;
}

void push_level(sprite_t** batch, level_t* level) {
  ASSERT(state.current_level < MAX_LEVELS && state.levels[state.current_level]);

  ASSERT(level->name, "must build the levels first");
  char level_msg[32];
  snprintf(level_msg, sizeof(level_msg), "%s <id=%d> (rooms=%d)", level->name,
           level->id, level->room_count);

  push_font_str(
      batch, &state.font_sheet, level_msg,
      (fv2){2 * state.font_sheet.sprite_width * state.font_sheet.scale,
            SCREEN_HEIGHT -
                (2 * state.font_sheet.sprite_height * state.font_sheet.scale)});

  for (int i = 0; i < level->room_count; ++i) {
    room_t* room = level->rooms[i];
    int sprite_sheet_width =
        room->tile_sheet.img.width / room->tile_sheet.sprite_width;
    f32 scale = room->tile_sheet.scale;
    for (int r_x = 0; r_x < room->w; ++r_x) {
      for (int r_y = 0; r_y < room->h; ++r_y) {
        int tile_value = room->tiles[r_y][r_x];
        // t = y * w + x
        int src_y = tile_value / sprite_sheet_width;
        int src_x = tile_value % sprite_sheet_width;
        *dynlist_append(*batch) = (sprite_t){
            .src_idx = (iv2){src_x, src_y},
            .dst_px =
                (fv2){(room->x + r_x) * room->tile_sheet.sprite_width * scale,
                      (room->y + r_y) * room->tile_sheet.sprite_height * scale},
            .rotation = (rand() % 4) * 90.0f,
            .src_sheet = &room->tile_sheet};
      }
    }
  }
}

void level_destroy(level_t* level) {
  for (int i = 0; level && i < level->room_count; ++i) {
    sprite_sheet_destroy(&level->rooms[i]->tile_sheet);
  }
}
