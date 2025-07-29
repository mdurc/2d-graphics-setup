#pragma once

#include "../physics/physics.h"
#include "../renderer/render.h"

typedef enum {
  COLLISION_LAYER_PLAYER = 1,
  COLLISION_LAYER_ENEMY = 1 << 1,
  COLLISION_LAYER_TERRAIN = 1 << 2,
  COLLISION_LAYER_KINEMATIC = 1 << 3,
} collision_layer_t;

// Physics Collision Callbacks
void player_on_hit(body_t* self, body_t* other, hit_t hit);
void player_on_hit_static(body_t* self, static_body_t* other, hit_t hit);
void enemy_on_hit_static(body_t* self, static_body_t* other, hit_t hit);
void kinematic_on_hit_static(body_t* self, static_body_t* other, hit_t hit);
void kinematic_on_hit(body_t* self, body_t* other, hit_t hit);

void setup_bodies_entities_anims(sprite_sheet_t* bg_sheet);

extern f32 player_size;
extern vec4 player_aabb_color;
extern bool player_is_grounded;

extern u8 enemy_mask;
extern u8 player_mask;
extern u8 kinematic_mask;

// storing the id's because the pointers could be invalidated by dynlist

// Static Body Ids
extern size_t sb_a_id;
extern size_t sb_b_id;
extern size_t sb_c_id;
extern size_t sb_d_id;
extern size_t sb_e_id;

// Kinematic Body Ids
extern size_t kin_id;

// Entity Ids
extern size_t e_player_id;
extern size_t e_a_id;
extern size_t e_b_id;

// Animation Ids
extern size_t anim_player_idle_id;
extern size_t anim_player_walk_id;
