#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "config/config.h"
#include "time.h"

typedef struct state { // for forward declaration
  GLFWwindow* window;

  config_t config;
  input_state_t input;
  time_state_t time;
} state_t;

void state_init(void);
void state_destroy(void);

extern state_t state;
