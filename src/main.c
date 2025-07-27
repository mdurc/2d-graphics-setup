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
static vec2 cursor_pos;

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
  cursor_pos[0] = x;
  cursor_pos[1] = SCREEN_HEIGHT - y;
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

  player_pos[0] = SCREEN_WIDTH * 0.5f;
  player_pos[1] = SCREEN_HEIGHT * 0.5f;

  aabb_t player_aabb = {.position = {SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5},
                        .half_size = {50, 50}};
  aabb_t cursor_aabb = {.half_size = {75, 75}};
  aabb_t sum_aabb = {
      .position = {player_aabb.position[0], player_aabb.position[1]},
      .half_size = {player_aabb.half_size[0] + cursor_aabb.half_size[0],
                    player_aabb.half_size[1] + cursor_aabb.half_size[1]}};
  aabb_t start_aabb = {.half_size = {75, 75}};

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

    // aabb rendering
    player_aabb.position[0] = player_pos[0];
    player_aabb.position[1] = player_pos[1];
    cursor_aabb.position[0] = cursor_pos[0];
    cursor_aabb.position[1] = cursor_pos[1];

    int left_mouse_state =
        glfwGetMouseButton(state.window, GLFW_MOUSE_BUTTON_LEFT);
    if (left_mouse_state == GLFW_PRESS) {
      start_aabb.position[0] = cursor_pos[0];
      start_aabb.position[1] = cursor_pos[1];
    }

    render_aabb((f32*)&player_aabb, WHITE);
    render_aabb((f32*)&sum_aabb, MAGENTA); // minkowski sum render

    aabb_t minkowski_diff = aabb_minkowski_difference(player_aabb, cursor_aabb);
    render_aabb((f32*)&minkowski_diff, ORANGE);
    // the usefulness comes from: our player box is intersecting with the cursor
    // box IFF the minkowski difference box contains the origin

    if (physics_aabb_intersect_aabb(player_aabb, cursor_aabb)) {
      vec2 pv;
      aabb_penetration_vector(pv, minkowski_diff);

      aabb_t collision_aabb = cursor_aabb;
      // move it outside of the player box the cursor is currently inside
      collision_aabb.position[0] += pv[0];
      collision_aabb.position[1] += pv[1];

      render_aabb((f32*)&cursor_aabb, RED);
      render_aabb((f32*)&collision_aabb, CYAN);

      // show the projection vector that we used
      vec2_add(pv, cursor_pos, pv);
      render_line_segment(cursor_pos, pv, CYAN);
    } else {
      render_aabb((f32*)&cursor_aabb, WHITE);
    }

    render_aabb((f32*)&start_aabb, GREEN);
    render_line_segment(start_aabb.position, cursor_pos, CYAN);

    if (physics_point_intersect_aabb(cursor_pos, player_aabb)) {
      render_quad(cursor_pos, (vec2){5, 5}, RED);
    } else {
      render_quad(cursor_pos, (vec2){5, 5}, WHITE);
    }

    render_end();
  }

  physics_destroy();
  state_destroy();
}
