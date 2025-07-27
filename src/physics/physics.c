#include "physics.h"

#include "../c-lib/dynlist.h"
#include "../c-lib/math.h"
#include "../state.h"

static physics_state_internal_t phys_state;
static u32 iterations = 2; // computation/accurate collisions
static f32 tick_rate;

void physics_init(void) {
  phys_state.body_list = dynlist_create(body_t);
  phys_state.static_body_list = dynlist_create(static_body_t);

  phys_state.gravity = -200;
  phys_state.terminal_velocity = -10000;
  tick_rate = 1.0f / iterations;
}
void physics_destroy(void) { dynlist_destroy(phys_state.body_list); }

static void update_sweep_result(hit_t* result, size_t other_id, aabb_t a,
                                aabb_t b, vec2 velocity, u8 a_collision_mask,
                                u8 b_collision_layer) {
  if ((a_collision_mask & b_collision_layer) == 0) {
    return;
  }

  aabb_t sum_aabb = b;
  vec2_add(sum_aabb.half_size, sum_aabb.half_size, a.half_size);

  hit_t hit = physics_ray_intersect_aabb(a.position, velocity, sum_aabb);
  if (hit.is_hit) {
    if (hit.time < result->time) {
      *result = hit;
    } else if (float_eq(hit.time, result->time)) {
      // get the highest velocity axis first
      if (fabsf(velocity[0]) > fabsf(velocity[1]) && hit.normal[0] != 0.0f) {
        *result = hit;
      } else if (fabsf(velocity[1]) > fabsf(velocity[0]) &&
                 hit.normal[1] != 0.0f) {
        *result = hit;
      }
    }
    result->other_id = other_id;
  }
}

static hit_t sweep_static_bodies(body_t* body, vec2 velocity) {
  hit_t result = {.time = 0xBBBB};

  for (u32 i = 0; i < dynlist_size(phys_state.static_body_list); ++i) {
    static_body_t* static_body = &phys_state.static_body_list[i];
    update_sweep_result(&result, i, body->aabb, static_body->aabb, velocity,
                        body->collision_mask, static_body->collision_layer);
  }

  return result;
}

static hit_t sweep_bodies(body_t* body, vec2 velocity) {
  hit_t result = {.time = 0xBBBB};

  for (u32 i = 0; i < dynlist_size(phys_state.body_list); ++i) {
    body_t* other = &phys_state.body_list[i];
    if (body == other) {
      continue;
    }
    update_sweep_result(&result, i, body->aabb, other->aabb, velocity,
                        body->collision_mask, other->collision_layer);
  }

  return result;
}

static void sweep_response(body_t* body, vec2 velocity) {
  hit_t hit = sweep_static_bodies(body, velocity);
  hit_t hit_moving = sweep_bodies(body, velocity);

  if (hit_moving.is_hit) {
    if (body->on_hit != NULL) {
      body->on_hit(body, physics_body_get(hit_moving.other_id), hit_moving);
    }
  }

  if (hit.is_hit) {
    body->aabb.position[0] = hit.position[0];
    body->aabb.position[1] = hit.position[1];

    if (hit.normal[0] != 0.0f) {
      body->aabb.position[1] += velocity[1];
      body->velocity[0] = 0;
    } else if (hit.normal[1] != 0.0f) {
      body->aabb.position[0] += velocity[0];
      body->velocity[1] = 0;
    }

    if (body->on_hit_static != NULL) {
      body->on_hit_static(body, physics_static_body_get(hit_moving.other_id),
                          hit);
    }
  } else {
    vec2_add(body->aabb.position, body->aabb.position, velocity);
  }
}

static void stationary_response(body_t* body) {
  dynlist_each(phys_state.static_body_list, static_body) {
    aabb_t aabb =
        physics_aabb_minkowski_difference(static_body->aabb, body->aabb);
    vec2 min, max;
    physics_aabb_min_max(min, max, aabb);

    if (min[0] <= 0 && max[0] >= 0 && min[1] <= 0 && max[1] >= 0) {
      vec2 penetration_vector;
      physics_aabb_penetration_vector(penetration_vector, aabb);

      vec2_add(body->aabb.position, body->aabb.position, penetration_vector);
    }
  }
}

void physics_update(void) {
  dynlist_each(phys_state.body_list, body) {
    body->velocity[1] += phys_state.gravity;
    if (phys_state.terminal_velocity > body->velocity[1]) {
      body->velocity[1] = phys_state.terminal_velocity;
    }

    body->velocity[0] += body->acceleration[0];
    body->velocity[1] += body->acceleration[1];

    // we are only doing narrow phase sweep, not a broad phase sweep, which may
    // be necessary for more performance
    vec2 scaled_velocity;
    vec2_scale(scaled_velocity, body->velocity, state.time.delta * tick_rate);
    for (u32 j = 0; j < iterations; ++j) {
      sweep_response(body, scaled_velocity);
      stationary_response(body);
      physics_clamp_body(body);
    }
  }
}

void physics_clamp_body(body_t* body) {
  f32 x = body->aabb.position[0];
  f32 y = body->aabb.position[1];
  body->aabb.position[0] = clamp(x, 0, SCREEN_WIDTH);
  body->aabb.position[1] = clamp(y, 0, SCREEN_WIDTH);

  if (!float_eq(body->aabb.position[0], x) ||
      !float_eq(body->aabb.position[1], y)) {
    LOG("CLAMPED body position");
  }
}

size_t physics_body_create(vec2 position, vec2 size, vec2 velocity,
                           u8 collision_layer, u8 collision_mask,
                           on_hit_func on_hit,
                           on_hit_static_func on_hit_static) {
  *dynlist_append(phys_state.body_list) = (body_t){
      .aabb = {.position = {position[0], position[1]},
               .half_size = {size[0] * 0.5f, size[1] * 0.5f}},
      .velocity = {velocity[0], velocity[1]},
      .acceleration = {0, 0},
      .collision_layer = collision_layer,
      .collision_mask = collision_mask,
      .on_hit = on_hit,
      .on_hit_static = on_hit_static,
  };

  return dynlist_size(phys_state.body_list) - 1;
}

body_t* physics_body_get(size_t idx) {
  ASSERT(idx < dynlist_size(phys_state.body_list));
  return &phys_state.body_list[idx];
}

size_t physics_static_body_create(vec2 position, vec2 size,
                                  u8 collision_layer) {
  *dynlist_append(phys_state.static_body_list) = (static_body_t){
      .aabb = {.position = {position[0], position[1]},
               .half_size = {size[0] * 0.5f, size[1] * 0.5f}},
      .collision_layer = collision_layer,
  };

  return dynlist_size(phys_state.static_body_list) - 1;
}

static_body_t* physics_static_body_get(size_t idx) {
  ASSERT(idx < dynlist_size(phys_state.static_body_list));
  return &phys_state.static_body_list[idx];
}

void physics_aabb_min_max(vec2 min, vec2 max, aabb_t aabb) {
  vec2_sub(min, aabb.position, aabb.half_size);
  vec2_add(max, aabb.position, aabb.half_size);
}

bool physics_point_intersect_aabb(vec2 point, aabb_t aabb) {
  vec2 min, max;
  physics_aabb_min_max(min, max, aabb);
  return point[0] >= min[0] && point[0] <= max[0] && point[1] >= min[1] &&
         point[1] <= max[1];
}

bool physics_aabb_intersect_aabb(aabb_t a, aabb_t b) {
  vec2 min, max;
  physics_aabb_min_max(min, max, physics_aabb_minkowski_difference(a, b));

  // check if the origin is within this aabb range
  return (min[0] <= 0 && max[0] >= 0 && min[1] <= 0 && max[1] >= 0);
}

aabb_t physics_aabb_minkowski_difference(aabb_t a, aabb_t b) {
  aabb_t result;
  vec2_sub(result.position, a.position, b.position);
  vec2_add(result.half_size, a.half_size, b.half_size);

  return result;
}

void physics_aabb_penetration_vector(vec2 r, aabb_t mink_aabb) {
  // the smallest vector that will separate the boxes
  // diagonals will never be the shortest, so we can check each side
  vec2 min, max;
  physics_aabb_min_max(min, max, mink_aabb);

  f32 min_dist = fabsf(min[0]); // distance the left edge is to the origin
  r[0] = min[0]; // initially push rightward (positive x) that amt
  r[1] = 0;

  // check if the right edge is closer
  if (fabsf(max[0]) < min_dist) {
    min_dist = fabsf(max[0]);
    r[0] = max[0]; // push leftward
  }

  // check the bottom edge
  if (fabsf(min[1]) < min_dist) {
    min_dist = fabsf(min[1]);
    r[0] = 0;
    r[1] = min[1]; // push upward
  }

  // check the top edge
  if (fabsf(max[1]) < min_dist) {
    r[0] = 0;
    r[1] = max[1]; // push downward
  }
}

hit_t physics_ray_intersect_aabb(vec2 pos, vec2 magnitude, aabb_t aabb) {
  hit_t hit = {0};
  vec2 min, max;
  physics_aabb_min_max(min, max, aabb);

  f32 last_entry = -INFINITY;
  f32 first_exit = INFINITY;

  for (u8 i = 0; i < 2; ++i) {
    if (magnitude[i] != 0.0f) {
      f32 t1 = (min[i] - pos[i]) / magnitude[i];
      f32 t2 = (max[i] - pos[i]) / magnitude[i];

      last_entry = fmaxf(last_entry, fminf(t1, t2));
      first_exit = fminf(first_exit, fmaxf(t1, t2));
    } else if (pos[i] <= min[i] || pos[i] >= max[i]) {
      return hit;
    }
  }

  if (first_exit > last_entry && first_exit > 0 && last_entry < 1) {
    hit.position[0] = pos[0] + magnitude[0] * last_entry;
    hit.position[1] = pos[1] + magnitude[1] * last_entry;

    hit.is_hit = true;
    hit.time = last_entry;

    f32 dx = hit.position[0] - aabb.position[0];
    f32 dy = hit.position[1] - aabb.position[1];
    f32 px = aabb.half_size[0] - fabsf(dx);
    f32 py = aabb.half_size[1] - fabsf(dy);

    if (px < py) {
      hit.normal[0] = (dx > 0) - (dx < 0);
    } else {
      hit.normal[1] = (dy > 0) - (dy < 0);
    }
  }

  return hit;
}
