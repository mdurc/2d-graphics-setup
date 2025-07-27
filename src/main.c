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

static void input_handle(body_t* player) {
  if (state.input.escape > 0) {
    glfwSetWindowShouldClose(state.window, true);
  }

  f64 x, y;
  glfwGetCursorPos(state.window, &x, &y);

  f32 velx = 0, vely = player->velocity[1];
  if (state.input.right > 0) {
    velx += 1000;
  }

  if (state.input.left > 0) {
    velx -= 1000;
  }

  if (state.input.up > 0) {
    vely = 4000;
  }

  if (state.input.down > 0) {
    vely -= 800;
  }

  player->velocity[0] = velx;
  player->velocity[1] = vely;
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

  size_t body_id = physics_body_create((vec2){640, 500}, (vec2){50, 50});

  f32 width = SCREEN_WIDTH, height = SCREEN_HEIGHT;
  size_t static_body_a_id = physics_static_body_create(
      (vec2){width * 0.5 - 25, height - 25}, (vec2){width - 50, 50});
  size_t static_body_b_id = physics_static_body_create(
      (vec2){width - 25, height * 0.5 + 25}, (vec2){50, height - 50});
  size_t static_body_c_id = physics_static_body_create(
      (vec2){width * 0.5 + 25, 25}, (vec2){width - 50, 50});
  size_t static_body_d_id = physics_static_body_create(
      (vec2){25, height * 0.5 - 25}, (vec2){50, height - 50});
  size_t static_body_e_id = physics_static_body_create(
      (vec2){width * 0.5, height * 0.5}, (vec2){150, 150});

  while (!glfwWindowShouldClose(state.window)) {
    time_update();

    // storing the body_id because the pointers could be invalidated by dynlist
    body_t* body_player = physics_body_get(body_id);
    static_body_t* static_body_a = physics_static_body_get(static_body_a_id);
    static_body_t* static_body_b = physics_static_body_get(static_body_b_id);
    static_body_t* static_body_c = physics_static_body_get(static_body_c_id);
    static_body_t* static_body_d = physics_static_body_get(static_body_d_id);
    static_body_t* static_body_e = physics_static_body_get(static_body_e_id);

    input_update();
    input_handle(body_player);
    physics_update();

    render_begin();

    //// two triangles
    // render_test_triangle(shader_temp, vao_one);
    // render_test_triangle(shader_temp, vao_two);
    //
    //// center quad
    // render_quad((vec2){SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f},
    //             (vec2){SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f},
    //             (vec4){0.0f, 0.0f, 1.0f, 1.0f});

    render_aabb((f32*)&static_body_a->aabb, WHITE);
    render_aabb((f32*)&static_body_b->aabb, WHITE);
    render_aabb((f32*)&static_body_c->aabb, WHITE);
    render_aabb((f32*)&static_body_d->aabb, WHITE);
    render_aabb((f32*)&static_body_e->aabb, WHITE);
    render_aabb((f32*)&body_player->aabb, WHITE);

    render_end();
    time_update_late();
  }

  physics_destroy();
  state_destroy();
}
