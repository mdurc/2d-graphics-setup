#include "background.h"
#include "c-lib/time.h"
#include "img.h"
#include "sprites.h"
#include "state.h"

state_t state;

int main(void) {
  initialize_state(&state, "window");
  sprites_init(&state.font_sheet, &state, "res/font.png", 8, 8, 3.0f);
  sprites_init(&state.bg_sheet, &state, "res/bg.png", 8, 8, 4.0f);

  build_levels();
  set_level(&state, 0);

  state.quit = false;
  SDL_Event ev;

  const char* msg =
      "abcdefghijklmnopqrstuvwxyz"
      "\nABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "\n0123456789"
      "\n!@#$%^&*()_+="
      "\n,./<>?;':\"[]";

  while (!state.quit) {
    while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
        case SDL_QUIT: state.quit = true; break;
      }
    }

    float t = 10.0f + (cosf(time_s()) * 10.0f);

    if ((t > 10.0f && state.current_level == 0) ||
        (t < 10.0f && state.current_level == 1)) {
      set_level(&state, state.current_level == 0 ? 1 : 0);
    }

    fv2 a_pos = {.x = t, .y = t};
    fv2 b_pos = {.x = t, .y = a_pos.y + 8.0f};

    push_font_ch(&state.font_sheet, 'A', a_pos);
    push_font_str(&state.font_sheet, msg, b_pos);
    push_background(&state.bg_sheet);
    push_level(&state);

    SDL_SetRenderDrawColor(state.renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(state.renderer);
    render_level(&state);
    render_batch(&state, &state.bg_sheet, true);
    render_batch(&state, &state.font_sheet, true);
    SDL_RenderPresent(state.renderer);
  }

  destroy_levels(&state);
  destroy_sheet(&state.font_sheet);
  destroy_sheet(&state.bg_sheet);
  SDL_DestroyRenderer(state.renderer);
  SDL_DestroyWindow(state.window);
  SDL_Quit();
}
