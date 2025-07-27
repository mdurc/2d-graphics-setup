#include "c-lib/dynlist.h"
#include "c-lib/math.h"
#include "c-lib/time.h"
#include "config/config.h"
#include "entity/entity.h"
#include "font.h"
#include "physics/physics.h"
#include "renderer/render.h"
#include "renderer/render_init.h"
#include "state.h"
#include "time.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

typedef enum collision_layer {
  COLLISION_LAYER_PLAYER = 1,
  COLLISION_LAYER_ENEMY = 1 << 1,
  COLLISION_LAYER_TERRAIN = 1 << 2,
} collision_layer_t;

vec4 player_color = {0, 1, 1, 1};
bool player_is_grounded = false;

void player_on_hit(body_t* self, body_t* other, hit_t hit);
void player_on_hit_static(body_t* self, static_body_t* other, hit_t hit);
void enemy_on_hit_static(body_t* self, static_body_t* other, hit_t hit);

static void input_handle(body_t* body_player) {
  if (state.input.escape > 0) {
    glfwSetWindowShouldClose(state.window, true);
  }

  f64 x, y;
  glfwGetCursorPos(state.window, &x, &y);

  f32 velx = 0, vely = body_player->velocity[1];
  if (state.input.right > 0) {
    velx += 1000;
  }

  if (state.input.left > 0) {
    velx -= 1000;
  }

  if (state.input.up > 0 && player_is_grounded) {
    vely = 4000;
    player_is_grounded = false;
  }

  if (state.input.down > 0) {
    vely -= 800;
  }

  body_player->velocity[0] = velx;
  body_player->velocity[1] = vely;
}

void player_on_hit(body_t* self, body_t* other, hit_t hit) {
  if (other->collision_layer == COLLISION_LAYER_ENEMY) {
    player_color[0] = 1;
    player_color[2] = 0;
  }
}

void player_on_hit_static(body_t* self, static_body_t* other, hit_t hit) {
  if (hit.normal[1] > 0) {
    player_is_grounded = true;
  }
}

void enemy_on_hit_static(body_t* self, static_body_t* other, hit_t hit) {
  if (hit.normal[0] > 0) {
    self->velocity[0] = 700;
  }

  if (hit.normal[0] < 0) {
    self->velocity[0] = -700;
  }
}

int main(void) {
  time_init(60);
  config_init();
  render_init(SCREEN_WIDTH, SCREEN_HEIGHT);
  physics_init();
  entity_init();
  state_init();

  u32 shader_temp, vao_one, vao_two;
  render_test_setup(&shader_temp, &vao_one, 1.0f);
  render_test_setup(&shader_temp, &vao_two, -1.0f);

  u8 enemy_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN;
  u8 player_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN;
  size_t player_id = entity_create(
      (vec2){550, 500}, (vec2){50, 50}, (vec2){0, 0}, COLLISION_LAYER_PLAYER,
      player_mask, player_on_hit, player_on_hit_static);

  f32 width = SCREEN_WIDTH, height = SCREEN_HEIGHT;
  size_t static_body_a_id = physics_static_body_create(
      (vec2){width * 0.5 - 25, height - 25}, (vec2){width - 50, 50},
      COLLISION_LAYER_TERRAIN);
  size_t static_body_b_id = physics_static_body_create(
      (vec2){width - 25, height * 0.5 + 25}, (vec2){50, height - 50},
      COLLISION_LAYER_TERRAIN);
  size_t static_body_c_id = physics_static_body_create(
      (vec2){width * 0.5 + 25, 25}, (vec2){width - 50, 50},
      COLLISION_LAYER_TERRAIN);
  size_t static_body_d_id = physics_static_body_create(
      (vec2){25, height * 0.5 - 25}, (vec2){50, height - 50},
      COLLISION_LAYER_TERRAIN);
  size_t static_body_e_id =
      physics_static_body_create((vec2){width * 0.5, height * 0.5},
                                 (vec2){150, 150}, COLLISION_LAYER_TERRAIN);

  size_t entity_a_id = entity_create((vec2){200, 600}, (vec2){25, 25},
                                     (vec2){900, 0}, COLLISION_LAYER_ENEMY,
                                     enemy_mask, NULL, enemy_on_hit_static);
  size_t entity_b_id = entity_create((vec2){500, 500}, (vec2){25, 25},
                                     (vec2){900, 0}, COLLISION_LAYER_ENEMY,
                                     enemy_mask, NULL, enemy_on_hit_static);

  while (!glfwWindowShouldClose(state.window)) {
    time_update();

    // storing the body_id because the pointers could be invalidated by dynlist
    entity_t* player = entity_get(player_id);
    body_t* body_player = physics_body_get(player->body_id);
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
    render_aabb((f32*)&body_player->aabb, player_color);

    render_aabb((f32*)physics_body_get(entity_get(entity_a_id)->body_id),
                WHITE);
    render_aabb((f32*)physics_body_get(entity_get(entity_b_id)->body_id),
                WHITE);

    render_end();

    player_color[0] = 0;
    player_color[2] = 1;
    time_update_late();
  }

  physics_destroy();
  state_destroy();
}
