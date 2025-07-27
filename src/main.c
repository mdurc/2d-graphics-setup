#include "c-lib/dynlist.h"
#include "c-lib/math.h"
#include "c-lib/time.h"
#include "config/config.h"
#include "font.h"
#include "physics/physics.h"
#include "renderer/render.h"
#include "renderer/render_init.h"
#include "state.h"
#include "time.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

static vec2 player_pos;
static input_key_t last_dir = INPUT_KEY_LEFT;
static int animation_idx = 1;

static void input_handle(void) {
  if (state.input.left == KS_PRESSED || state.input.left == KS_HELD) {
    player_pos[0] -= 500 * state.time.delta;
    last_dir = INPUT_KEY_LEFT;
    animation_idx = 1;
  }
  if (state.input.right == KS_PRESSED || state.input.right == KS_HELD) {
    player_pos[0] += 500 * state.time.delta;
    last_dir = INPUT_KEY_RIGHT;
    animation_idx = 2;
  }
  if (state.input.up == KS_PRESSED || state.input.up == KS_HELD) {
    player_pos[1] -= 500 * state.time.delta;
    animation_idx = last_dir == INPUT_KEY_LEFT ? 3 : 4;
  }
  if (state.input.down == KS_PRESSED || state.input.down == KS_HELD) {
    player_pos[1] += 500 * state.time.delta;
    animation_idx = last_dir == INPUT_KEY_LEFT ? 5 : 6;
  }
  if (state.input.escape == KS_PRESSED || state.input.escape == KS_HELD) {
    glfwSetWindowShouldClose(state.window, true);
  }
}

int main(void) {
  time_init(60);
  config_init();
  render_init(SCREEN_WIDTH, SCREEN_HEIGHT);
  physics_init();
  state_init();

  int body_count = 500;
  for (int i = 0; i < body_count; ++i) {
    size_t idx = physics_body_create(
        (vec2){rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT},
        (vec2){rand() % 100, rand() % 100});
    body_t* body = physics_body_get(idx);
    body->acceleration[0] = rand() % 200 - 100;
    body->acceleration[1] = rand() % 200 - 100;
  }

  u32 shader_temp, vao_one, vao_two;
  render_test_setup(&shader_temp, &vao_one, 1.0f);
  render_test_setup(&shader_temp, &vao_two, -1.0f);

  player_pos[0] = SCREEN_WIDTH / 2.0f - 100;
  player_pos[1] = SCREEN_HEIGHT / 2.0f - 100;

  const char* msg =
      "abcdefghijklmnopqrstuvwxyz"
      "\nABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "\n0123456789"
      "\n!@#$%^&*()_+="
      "\n,./<>?;':\"[]";

  while (!glfwWindowShouldClose(state.window)) {
    time_update();

    input_update();
    input_handle();
    physics_update();

    // f32 t = 100.0f + (cosf(time_s()) * 100.0f);

    render_begin();
    glUseProgram(shader_temp);
    render_test_triangle(shader_temp, vao_one);
    render_test_triangle(shader_temp, vao_two);
    render_quad((vec2){SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f},
                (vec2){SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f},
                (vec4){0.0f, 0.0f, 1.0f, 1.0f});

    for (int i = 0; i < body_count; ++i) {
      body_t* body = physics_body_get(i);
      render_quad_lines(body->aabb.position, body->aabb.half_size,
                        (vec4){1.0f, 0.0f, 0.0f, 1.0f});

      float px = body->aabb.position[0];
      float py = body->aabb.position[1];

      if (px < 0 || px > SCREEN_WIDTH) {
        body->aabb.position[0] = clamp(px, 0, SCREEN_WIDTH);
        body->velocity[0] *= -1;
      }

      if (py < 0 || py > SCREEN_HEIGHT) {
        body->aabb.position[1] = clamp(py, 0, SCREEN_HEIGHT);
        body->velocity[1] *= -1;
      }
      body->velocity[0] = clamp(body->velocity[0], -500.0f, 500.0f);
      body->velocity[1] = clamp(body->velocity[1], -500.0f, 500.0f);
    }

    render_end();
  }

  state_destroy();
}
