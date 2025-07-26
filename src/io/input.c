#include "input.h"

#include "../c-lib/types.h"
#include "../state.h"
#include "SDL_keyboard.h"

static void update_key_state(u8 current_state, key_state_t* key_state) {
  if (current_state) {
    if (*key_state > 0) {
      *key_state = KS_HELD;
    } else {
      *key_state = KS_PRESSED;
    }
  } else {
    *key_state = KS_UNPRESSED;
  }
}

void input_update(void) {
  const u8* keyboard_state = SDL_GetKeyboardState(NULL);

  update_key_state(keyboard_state[state.config.keybinds[INPUT_KEY_LEFT]],
                   &state.input.left);
  update_key_state(keyboard_state[state.config.keybinds[INPUT_KEY_RIGHT]],
                   &state.input.right);
  update_key_state(keyboard_state[state.config.keybinds[INPUT_KEY_UP]],
                   &state.input.up);
  update_key_state(keyboard_state[state.config.keybinds[INPUT_KEY_DOWN]],
                   &state.input.down);
  update_key_state(keyboard_state[state.config.keybinds[INPUT_KEY_ESCAPE]],
                   &state.input.escape);
}
