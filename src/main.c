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
static vec2 mouse_pos;

static input_key_t last_dir = INPUT_KEY_LEFT;
static int animation_idx = 1;

static void check_player_move(void) {
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
    player_pos[1] += 500 * state.time.delta;
    animation_idx = last_dir == INPUT_KEY_LEFT ? 3 : 4;
  }
  if (state.input.down == KS_PRESSED || state.input.down == KS_HELD) {
    player_pos[1] -= 500 * state.time.delta;
    animation_idx = last_dir == INPUT_KEY_LEFT ? 5 : 6;
  }
}

static void input_handle(void) {
  if (state.input.escape == KS_PRESSED || state.input.escape == KS_HELD) {
    glfwSetWindowShouldClose(state.window, true);
  }
  check_player_move();

  f64 x, y;
  glfwGetCursorPos(state.window, &x, &y);
  mouse_pos[0] = x;
  mouse_pos[1] = SCREEN_HEIGHT - y;
}

int main(void) {
  time_init(60);
  config_init();
  render_init(SCREEN_WIDTH, SCREEN_HEIGHT);
  physics_init();
  state_init();

  u32 shader_temp, vao_one, vao_two;
  render_test_setup(&shader_temp, &vao_one, 1.0f);
  render_test_setup(&shader_temp, &vao_two, -1.0f);

  player_pos[0] = SCREEN_WIDTH / 2.0f - 100;
  player_pos[1] = SCREEN_HEIGHT / 2.0f - 100;

  size_t idx = physics_body_create(player_pos, (vec2){100, 100});
  body_t* body = physics_body_get(idx);

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

    // two triangles
    render_test_triangle(shader_temp, vao_one);
    render_test_triangle(shader_temp, vao_two);

    // center quad
    render_quad((vec2){SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f},
                (vec2){SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f},
                (vec4){0.0f, 0.0f, 1.0f, 1.0f});

    // moveable quad lines
    render_aabb((f32*)&body->aabb, BLACK);
    body->aabb.position[0] = player_pos[0];
    body->aabb.position[1] = player_pos[1];

    // mouse quad
    if (physics_point_intersect_aabb(mouse_pos, body->aabb)) {
      render_quad(mouse_pos, (vec2){5, 5}, RED);
    } else {
      render_quad(mouse_pos, (vec2){5, 5}, WHITE);
    }

    render_end();
  }

  state_destroy();
}
