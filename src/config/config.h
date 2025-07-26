#pragma once

#include "../c-lib/types.h"
#include "../io/input.h"

typedef struct {
  u8 keybinds[5];
} config_t;

void config_init(void);
void config_key_bind(input_key_t key, const char* key_name);
