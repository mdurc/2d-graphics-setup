#pragma once

#include "../c-lib/types.h"
#include "../io/input.h"

#define INPUT_KEY_COUNT 256

typedef struct {
  u32 keybinds[INPUT_KEY_COUNT];
} config_t;

void config_init(void);
void config_key_bind(input_key_t key, const char* key_name);
