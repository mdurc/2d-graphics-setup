#include "data.h"

#include "../animation/animation.h"
#include "../entity/entity.h"
#include "../physics/physics.h"
#include "../renderer/render.h"
#include "../c-lib/log.h"

f32 player_size = 36;
vec4 player_aabb_color = {0, 1, 1, 1};
bool player_is_grounded = false;

u8 enemy_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN;
u8 player_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN;
u8 kinematic_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN | COLLISION_LAYER_PLAYER;

// Static Body Ids
size_t sb_a_id;
size_t sb_b_id;
size_t sb_c_id;
size_t sb_d_id;
size_t sb_e_id;

// Kinematic Body Ids
size_t kin_id;

// Entity Ids
size_t e_player_id;
size_t e_a_id;
size_t e_b_id;

// Animation Ids
size_t anim_player_idle_id;
size_t anim_player_walk_id;

void player_on_hit(body_t* self, body_t* other, hit_t hit) {
  (void)self;
  (void)hit;
  if (other->collision_layer == COLLISION_LAYER_ENEMY) {
    player_aabb_color[0] = 1;
    player_aabb_color[2] = 0;
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
    self->velocity[0] = 400;
  }

  if (hit.normal[0] < 0) {
    self->velocity[0] = -400;
  }
}

void kinematic_on_hit_static(body_t* self, static_body_t* other, hit_t hit) {
  (void)self;
  (void)other;
  (void)hit;
   LOG("Kinematic body colliding with static body");
}

void kinematic_on_hit(body_t* self, body_t* other, hit_t hit) {
  (void)self;
  (void)other;
  (void)hit;
  if (other->collision_layer == COLLISION_LAYER_PLAYER) {
    LOG("Kinematic body colliding with player body");
  } else if (other->collision_layer == COLLISION_LAYER_ENEMY) {
    LOG("Kinematic body colliding with enemy body");
  }
}

void setup_bodies_entities_anims(sprite_sheet_t* bg_sheet) {
  fv2 render_size = render_get_render_size();
  f32 render_scale = render_get_render_scale();

  f32 render_width = render_size.x;
  f32 render_height = render_size.y;
  f32 margin = 12; // from the center
  f32 thickness = player_size / render_scale;

  sb_a_id = physics_static_body_create((vec2){render_width * 0.5, render_height - margin}, (vec2){render_width - 1, thickness}, COLLISION_LAYER_TERRAIN); // top border
  sb_b_id = physics_static_body_create((vec2){render_width - margin, render_height * 0.5}, (vec2){thickness, render_height - 1}, COLLISION_LAYER_TERRAIN); // left border
  sb_c_id = physics_static_body_create((vec2){render_width * 0.5, margin}, (vec2){render_width - 1, thickness}, COLLISION_LAYER_TERRAIN); // bottom border
  sb_d_id = physics_static_body_create((vec2){margin, render_height * 0.5}, (vec2){thickness, render_height - 1}, COLLISION_LAYER_TERRAIN); // left border
  sb_e_id = physics_static_body_create((vec2){render_width * 0.5, render_height * 0.5}, (vec2){player_size * 1.5, player_size * 1.5}, COLLISION_LAYER_TERRAIN); // center box
  //kin_id = physics_body_create((vec2){render_width * 0.85, render_height * 0.5}, (vec2){player_size * 2, player_size * 3}, (vec2){-20.f, 0}, COLLISION_LAYER_KINEMATIC, kinematic_mask, true, kinematic_on_hit, kinematic_on_hit_static);

  vec2 enemy_size = (vec2){player_size * 0.5, player_size * 0.5};
  e_player_id = entity_create((vec2){render_width * 0.5f - player_size * 3, render_height * 0.5}, (vec2){player_size, player_size}, (vec2){0, 0}, COLLISION_LAYER_PLAYER, player_mask, false, (size_t)-1, player_on_hit, player_on_hit_static);
  //e_a_id = entity_create((vec2){render_width * 0.5 - 50, render_height * 0.5}, enemy_size, (vec2){400, 0}, COLLISION_LAYER_ENEMY, enemy_mask, false, (size_t)-1, NULL, enemy_on_hit_static);
  //e_b_id = entity_create((vec2){render_width * 0.5 + 50, render_height * 0.5}, enemy_size, (vec2){400, 0}, COLLISION_LAYER_ENEMY, enemy_mask, false, (size_t)-1, NULL, enemy_on_hit_static);

  size_t player_idle_def = animation_definition_create(bg_sheet, 0.2f, 1, (u8[]){0, 1}, 2);
  size_t player_walk_def = animation_definition_create(bg_sheet, 0.2f, 1, (u8[]){2, 3}, 2);
  anim_player_idle_id = animation_create(player_idle_def, true);
  anim_player_walk_id = animation_create(player_walk_def, true);
}
