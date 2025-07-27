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

    vec4 faded = {1, 1, 1, 0.3f};

    if (physics_aabb_intersect_aabb(player_aabb, cursor_aabb)) {
      render_aabb((f32*)&cursor_aabb, RED);
    } else {
      render_aabb((f32*)&cursor_aabb, WHITE);
    }

    render_aabb((f32*)&start_aabb, faded);
    render_line_segment(start_aabb.position, cursor_pos, faded);

    f32 x = sum_aabb.position[0];
    f32 y = sum_aabb.position[1];
    f32 size = sum_aabb.half_size[0];

    render_line_segment((vec2){x-size, 0}, (vec2){x-size, SCREEN_HEIGHT}, faded);
    render_line_segment((vec2){x+size, 0}, (vec2){x+size, SCREEN_HEIGHT}, faded);
    render_line_segment((vec2){0, y-size}, (vec2){SCREEN_WIDTH, y-size}, faded);
    render_line_segment((vec2){0, y+size}, (vec2){SCREEN_WIDTH, y+size}, faded);

    vec2 min, max;
    physics_aabb_min_max(min, max, sum_aabb);

    vec2 magnitude;
    vec2_sub(magnitude, cursor_pos, start_aabb.position);

    hit_t hit = physics_ray_intersect_aabb(start_aabb.position, magnitude, sum_aabb);

    if (hit.is_hit) {
      aabb_t hit_aabb = {
        .position = {hit.position[0], hit.position[1]},
        .half_size = {start_aabb.half_size[0], start_aabb.half_size[1]}
      };
      render_aabb((f32*)&hit_aabb, CYAN);
      render_quad(hit.position, (vec2){5,5}, CYAN);
    }

    for (u8 i = 0; i < 2; ++i) {
      if (magnitude[i] != 0.0f) {
        f32 t1 = (min[i] - cursor_pos[i]) / magnitude[i];
        f32 t2 = (max[i] - cursor_pos[i]) / magnitude[i];

        // entry points are cyan and exit points are orange.
        // we know that if our box goes within the sum_aabb, then the box is colliding.
        // thus, if our ray-path shows that our box is going to enter the sum_aabb box
        // and then exit the sum_aabb box, then we know that the path leads to a collision

        vec2 point;
        vec2_scale(point, magnitude, t1);
        vec2_add(point, point, cursor_pos);
        if (min[i] < start_aabb.position[i]) {
          render_quad(point, (vec2){5, 5}, ORANGE);
        } else {
          render_quad(point, (vec2){5, 5}, CYAN);
        }

        vec2_scale(point, magnitude, t2);
        vec2_add(point, point, cursor_pos);
        if (max[i] < start_aabb.position[i]) {
          render_quad(point, (vec2){5, 5}, CYAN);
        } else {
          render_quad(point, (vec2){5, 5}, ORANGE);
        }
      }
    }

    render_end();
    time_update_late();
  }

  physics_destroy();
  state_destroy();
}
