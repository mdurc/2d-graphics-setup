#include "c-lib/time.h"
#include "img.h"
#include "sprites.h"
#include "state.h"

state_t state;

int main() {
  initialize_state(&state, "window");
  sprites_init(&state.font_sheet, &state, "res/font.png", 8, 8);

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

    int t = 100 + (int)(cosf(time_s()) * 100);
    iv2 c_pos = {.x = t, .y = t};
    iv2 s_pos = {.x = t, .y = t + 9 * state.font_sheet.scale};

    font_ch(&state.font_sheet, 'A', &c_pos);
    font_str(&state.font_sheet, msg, &s_pos);
    load_batch(&state, &state.font_sheet, true);
  }

  destroy_img(&state.font_sheet.img);
  SDL_DestroyRenderer(state.renderer);
  SDL_DestroyWindow(state.window);
  SDL_Quit();
}
