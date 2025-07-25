#include "render.h"

#include "../c-lib/dynlist.h"
#include "../c-lib/misc.h"
#include "../state.h"

void render_init(u32 width, u32 height) {
  ASSERT(!SDL_Init(SDL_INIT_VIDEO), "failed SDL_Init: %s\n", SDL_GetError());

  state.window =
      SDL_CreateWindow("window name", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
  ASSERT(state.window, "failed to create sdl window: %s\n", SDL_GetError());

  state.renderer = SDL_CreateRenderer(
      state.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  ASSERT(state.renderer, "failed to create sdl renderer: %s\n", SDL_GetError());
}

void render_begin(void) {
  SDL_SetRenderDrawColor(state.renderer, 0x00, 0x00, 0x00, 0xFF);
  SDL_RenderClear(state.renderer);
}

void render_end(void) { SDL_RenderPresent(state.renderer); }

void render_batch(sprite_t** batch, bool clear_after_render) {
  ASSERT(batch, "dynlist batch must not be null");

  dynlist_each(*batch, sprite) {
    ASSERT(sprite->src_sheet, "src sprite sheet cannot be null for a sprite");
    int sw = sprite->src_sheet->sprite_width;
    int sh = sprite->src_sheet->sprite_height;
    f32 scale = sprite->src_sheet->scale;
    SDL_Rect src = {.x = sprite->src_idx.x * sw,
                    .y = sprite->src_idx.y * sh,
                    .w = sw,
                    .h = sh};
    SDL_Rect dst_px_pos = {.x = (int)(sprite->dst_px.x * scale),
                           .y = (int)(sprite->dst_px.y * scale),
                           .w = (int)(sw * scale),
                           .h = (int)(sh * scale)};
    SDL_RenderCopyEx(state.renderer, sprite->src_sheet->img.texture, &src,
                     &dst_px_pos, sprite->rotation, NULL, SDL_FLIP_NONE);
  }

  if (clear_after_render) {
    dynlist_clear(batch);
  }
}
