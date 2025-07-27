#include "state.h"

state_t state;

void state_init(void) {
  // sprite_sheet_init(&state.font_sheet, "res/font.png", 8, 8, 3.0f);
  // sprite_sheet_init(&state.bg_sheet, "res/bg.png", 8, 8, 4.0f);
}

void state_destroy(void) {
  // sprite_sheet_destroy(&state.font_sheet);
  // sprite_sheet_destroy(&state.bg_sheet);

  SDL_DestroyRenderer(state.renderer);
  glfwTerminate();
  SDL_Quit();
}
