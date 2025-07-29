#include "animation/animation.h"
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
  COLLISION_LAYER_KINEMATIC = 1 << 3,
} collision_layer_t;

f32 scale = 3.0f;

vec4 player_color = {0, 1, 1, 1};
bool player_is_grounded = false;

void player_on_hit(body_t* self, body_t* other, hit_t hit);
void player_on_hit_static(body_t* self, static_body_t* other, hit_t hit);
void enemy_on_hit_static(body_t* self, static_body_t* other, hit_t hit);
void kinematic_on_hit_static(body_t* self, static_body_t* other, hit_t hit);
void kinematic_on_hit(body_t* self, body_t* other, hit_t hit);

static void input_handle(body_t* body_player) {
  if (state.input.escape > 0) {
    glfwSetWindowShouldClose(state.window, true);
  }

  f64 x, y;
  glfwGetCursorPos(state.window, &x, &y);

  f32 velx = 0, vely = body_player->velocity[1];
  if (state.input.right > 0) {
    velx += 500 / scale;
  }

  if (state.input.left > 0) {
    velx -= 500 / scale;
  }

  if (state.input.up > 0 && player_is_grounded) {
    vely = 4000 / scale;
    player_is_grounded = false;
  }

  body_player->velocity[0] = velx;
  body_player->velocity[1] = vely;
}

void player_on_hit(body_t* self, body_t* other, hit_t hit) {
  (void)self;
  (void)hit;
  if (other->collision_layer == COLLISION_LAYER_ENEMY) {
    player_color[0] = 1;
    player_color[2] = 0;
  }
}

void player_on_hit_static(body_t* self, static_body_t* other, hit_t hit) {
  (void)self;
  (void)other;
  if (hit.normal[1] > 0) {
    player_is_grounded = true;
  }
}

void enemy_on_hit_static(body_t* self, static_body_t* other, hit_t hit) {
  (void)other;
  if (hit.normal[0] > 0) {
    self->velocity[0] = 700 / scale;
  }

  if (hit.normal[0] < 0) {
    self->velocity[0] = -700 / scale;
  }
}

void kinematic_on_hit_static(body_t* self, static_body_t* other, hit_t hit) {
  (void)self;
  (void)other;
  (void)hit;
  // LOG("KINEMATIC IS COLLIDING WITH STATIC BODY");
}

void kinematic_on_hit(body_t* self, body_t* other, hit_t hit) {
  (void)self;
  (void)other;
  (void)hit;
  // LOG("BODY COLLIDED WITH THE KINEMATIC");
}

int main(void) {
  time_init(60);
  config_init();
  render_init(1280, 720, scale);
  physics_init();
  entity_init();
  animation_init();
  state_init();

  u32 shader_temp, vao_one, vao_two;
  render_test_setup(&shader_temp, &vao_one, 1.0f);
  render_test_setup(&shader_temp, &vao_two, -1.0f);

  u8 enemy_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN;
  u8 player_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN;
  u8 kinematic_mask =
      COLLISION_LAYER_ENEMY | COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN;

  fv2 render_size = render_get_render_size();
  f32 width = render_size.x;
  f32 height = render_size.y;
  f32 player_size = 36;
  f32 margin = 12; // from the center
  f32 thickness = 36 / scale;

  size_t player_id = entity_create(
      (vec2){width * 0.5f + player_size * 3, height * 0.5},
      (vec2){player_size, player_size}, (vec2){0, 0}, COLLISION_LAYER_PLAYER,
      player_mask, false, (size_t)-1, player_on_hit, player_on_hit_static);
  size_t kinematic_body_id = physics_body_create(
      (vec2){width * 0.85, height * 0.5},
      (vec2){player_size * 2, player_size * 3}, (vec2){-10.f, 0},
      COLLISION_LAYER_KINEMATIC, kinematic_mask, true, kinematic_on_hit,
      kinematic_on_hit_static);
  size_t static_body_a_id = physics_static_body_create(
      (vec2){width * 0.5, height - margin}, (vec2){width - 1, thickness},
      COLLISION_LAYER_TERRAIN); // top border
  size_t static_body_b_id = physics_static_body_create(
      (vec2){width - margin, height * 0.5}, (vec2){thickness, height - 1},
      COLLISION_LAYER_TERRAIN); // left border
  size_t static_body_c_id = physics_static_body_create(
      (vec2){width * 0.5, margin}, (vec2){width - 1, thickness},
      COLLISION_LAYER_TERRAIN); // bottom border
  size_t static_body_d_id = physics_static_body_create(
      (vec2){margin, height * 0.5}, (vec2){thickness, height - 1},
      COLLISION_LAYER_TERRAIN); // left border
  size_t static_body_e_id =
      physics_static_body_create((vec2){width * 0.5, height * 0.5},
                                 (vec2){player_size * 1.5, player_size * 1.5},
                                 COLLISION_LAYER_TERRAIN); // center box

  vec2 enemy_size = (vec2){player_size * 0.5, player_size * 0.5};
  size_t entity_a_id =
      entity_create((vec2){width * 0.5 - 50, height * 0.5}, enemy_size,
                    (vec2){400, 0}, COLLISION_LAYER_ENEMY, enemy_mask, false,
                    (size_t)-1, NULL, enemy_on_hit_static);
  size_t entity_b_id =
      entity_create((vec2){width * 0.5 + 50, height * 0.5}, enemy_size,
                    (vec2){400, 0}, COLLISION_LAYER_ENEMY, enemy_mask, false,
                    (size_t)-1, NULL, enemy_on_hit_static);

  sprite_sheet_t bg_sheet;
  render_init_sprite_sheet(&bg_sheet, "./res/bg.png", 8, 8);
  sprite_sheet_t font_sheet;
  render_init_sprite_sheet(&font_sheet, "./res/font.png", 8, 8);

  size_t player_def_idle_id =
      animation_definition_create(&bg_sheet, 0.2f, 1, (u8[]){0, 1}, 2);
  size_t player_def_walk_id =
      animation_definition_create(&bg_sheet, 0.2f, 1, (u8[]){2, 3}, 2);
  size_t player_anim_idle_id = animation_create(player_def_idle_id, true);
  size_t player_anim_walk_id = animation_create(player_def_walk_id, true);

  const char* msg =
      "abcdefghijklmnopqrstuvwxyz"
      "\nABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "\n0123456789"
      "\n!@#$%^&*()_+="
      "\n,./<>?;':\"[]";

  while (!glfwWindowShouldClose(state.window)) {
    time_update();

    // storing the body_id because the pointers could be invalidated by dynlist
    entity_t* player = entity_get(player_id);
    body_t* body_player = physics_body_get(player->body_id);

    if (body_player->velocity[0] != 0.0f) {
      player->animation_id = player_anim_walk_id;
    } else {
      player->animation_id = player_anim_idle_id;
    }

    body_t* kinematic_body = physics_body_get(kinematic_body_id);
    static_body_t* static_body_a = physics_static_body_get(static_body_a_id);
    static_body_t* static_body_b = physics_static_body_get(static_body_b_id);
    static_body_t* static_body_c = physics_static_body_get(static_body_c_id);
    static_body_t* static_body_d = physics_static_body_get(static_body_d_id);
    static_body_t* static_body_e = physics_static_body_get(static_body_e_id);

    input_update();
    input_handle(body_player);
    physics_update();
    animation_update(state.time.delta);

    render_begin();

    // two triangles
    render_test_triangle(shader_temp, vao_one);
    render_test_triangle(shader_temp, vao_two);

    render_aabb((f32*)&kinematic_body->aabb, WHITE);
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

    for (size_t i = 0; i < entity_count(); ++i) {
      entity_t* entity = entity_get(i);
      if (entity->animation_id == (size_t)-1) {
        continue;
      }
      body_t* body = physics_body_get(entity->body_id);
      animation_t* anim = animation_get(entity->animation_id);
      if (body->velocity[0] < 0) {
        anim->is_flipped = true;
      } else if (body->velocity[0] > 0) {
        anim->is_flipped = false;
      }
      animation_render(anim, body->aabb.position,
                       (vec2){player_size, player_size}, WHITE);
    }

    float t = 25.0f + (cosf(time_s()) * 15.0f);
    font_render_str(&font_sheet, msg, (vec2){t, height - t}, NULL, TURQUOISE);

    render_end();

    player_color[0] = 0;
    player_color[2] = 1;
    time_update_late();
  }

  physics_destroy();
  state_destroy();
}
