#pragma once

typedef enum {
  INPUT_KEY_LEFT,
  INPUT_KEY_RIGHT,
  INPUT_KEY_UP,
  INPUT_KEY_DOWN,
  INPUT_KEY_ESCAPE
} input_key_t;

typedef enum {
  KS_UNPRESSED,
  KS_PRESSED,
  KS_HELD,
} key_state_t;

typedef struct {
  key_state_t left, right, up, down, escape;
} input_state_t;

void input_update(void);
