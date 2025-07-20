#include "background.h"
#include "c-lib/time.h"
#include "img.h"
#include "sprites.h"
#include "state.h"

state_t state;

int main() {
  initialize_state(&state, "window");
  sprites_init(&state.font_sheet, &state, "res/font.png", 8, 8);
  sprites_init(&state.bg_sheet, &state, "res/bg.png", 8, 8);

  state.quit = false;
  SDL_Event ev;

  const char* msg =
      "abcdefghijklmnopqrstuvwxyz"
      "\nABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "\n0123456789"
      "\n!@#$%^&*()_+="
      "\n,./<>?;':\"[]";

  iv2* coords = generate_coords(16, 16);

  while (!state.quit) {
    while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
        case SDL_QUIT: state.quit = true; break;
      }
    }

    float t = 10.0f + (cosf(time_s()) * 10.0f);
    fv2 a_pos = {.x = t, .y = t};
    fv2 b_pos = {.x = t, .y = a_pos.y + 8.0f};

    font_ch(&state.font_sheet, 'A', a_pos);
    font_str(&state.font_sheet, msg, b_pos);
    add_background(&state.bg_sheet, coords, 256);

    SDL_SetRenderDrawColor(state.renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(state.renderer);
    load_batch(&state, &state.bg_sheet, true);
    load_batch(&state, &state.font_sheet, true);
    SDL_RenderPresent(state.renderer);
  }

  free(coords);
  destroy_sheet(&state.font_sheet);
  destroy_sheet(&state.bg_sheet);
  SDL_DestroyRenderer(state.renderer);
  SDL_DestroyWindow(state.window);
  SDL_Quit();
}
