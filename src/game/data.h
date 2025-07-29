#pragma once

#include "../physics/physics.h"
#include "../renderer/render.h"

typedef enum {
  COLLISION_LAYER_PLAYER = 1,
  COLLISION_LAYER_ENEMY = 1 << 1,
  COLLISION_LAYER_TERRAIN = 1 << 2,
  COLLISION_LAYER_KINEMATIC = 1 << 3,
} collision_layer_t;

void player_on_hit(body_t* self, body_t* other, hit_t hit);
void player_on_hit_static(body_t* self, static_body_t* other, hit_t hit);
void enemy_on_hit_static(body_t* self, static_body_t* other, hit_t hit);
void kinematic_on_hit_static(body_t* self, static_body_t* other, hit_t hit);
void kinematic_on_hit(body_t* self, body_t* other, hit_t hit);

void setup(sprite_sheet_t* bg_sheet);

extern f32 player_size;
extern vec4 player_color;
extern bool player_is_grounded;

extern u8 enemy_mask;
extern u8 player_mask;
extern u8 kinematic_mask;

// storing the id's because the pointers could be invalidated by dynlist
extern size_t static_body_a_id;
extern size_t static_body_b_id;
extern size_t static_body_c_id;
extern size_t static_body_d_id;
extern size_t static_body_e_id;

extern size_t entity_player_id;
extern size_t entity_a_id;
extern size_t entity_b_id;
extern size_t kinematic_body_id;

extern size_t player_def_idle_id;
extern size_t player_def_walk_id;
extern size_t player_anim_idle_id;
extern size_t player_anim_walk_id;
