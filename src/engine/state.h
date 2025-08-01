#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "config/config.h"
#include "time/time.h"

typedef struct state { // for forward declaration
  GLFWwindow* window;

  config_t config;
  input_state_t input;
  time_state_t time;
} state_t;

extern state_t state;
