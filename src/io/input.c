#include "input.h"

#include "../state.h"

#define MAX_KEYS GLFW_KEY_LAST + 1
static bool key_current[MAX_KEYS] = {0};
static bool key_previous[MAX_KEYS] = {0};

static void update_key_state(bool current_state, key_state_t* key_state) {
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
  glfwPollEvents();

  memcpy(key_previous, key_current, sizeof(key_current));
  for (u32 key = 0; key < MAX_KEYS; ++key) {
    key_current[key] = glfwGetKey(state.window, key) == GLFW_PRESS;
  }

  update_key_state(key_current[state.config.keybinds[INPUT_KEY_LEFT]],
                   &state.input.left);
  update_key_state(key_current[state.config.keybinds[INPUT_KEY_RIGHT]],
                   &state.input.right);
  update_key_state(key_current[state.config.keybinds[INPUT_KEY_UP]],
                   &state.input.up);
  update_key_state(key_current[state.config.keybinds[INPUT_KEY_DOWN]],
                   &state.input.down);
  update_key_state(key_current[state.config.keybinds[INPUT_KEY_ESCAPE]],
                   &state.input.escape);
  update_key_state(key_current[state.config.keybinds[INPUT_KEY_DEBUG]],
                   &state.input.debug);
}
