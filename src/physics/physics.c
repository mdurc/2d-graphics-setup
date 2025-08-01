#include "physics.h"

#include "../c-lib/dynlist.h"
#include "../c-lib/log.h"
#include "../c-lib/math.h"
#include "../renderer/render.h"

static f32 gravity;
static f32 terminal_velocity;
static DYNLIST(body_t) body_list;
static DYNLIST(static_body_t) static_body_list;
static u32 iterations = 2; // computation/accurate collisions
static f32 tick_rate;

void physics_init(void) {
  body_list = dynlist_create(body_t);
  static_body_list = dynlist_create(static_body_t);

  gravity = -100;
  terminal_velocity = -7000;
  tick_rate = 1.0f / iterations;
  LOG("Physics system initialized");
}

void physics_destroy(void) {
  dynlist_destroy(body_list);
  dynlist_destroy(static_body_list);
  LOG("Physics system deinitialized");
}

void physics_deactivate(size_t idx) {
  physics_body_get(idx)->is_active = false;
}

f32* physics_get_gravity(void) { return &gravity; }
f32* physics_get_terminal_velocity(void) { return &terminal_velocity; }

static void update_sweep_result(hit_t* result, aabb_t a, aabb_t b,
                                size_t other_id, vec2 velocity,
                                u8 a_collision_mask, u8 b_collision_layer) {
  if ((a_collision_mask & b_collision_layer) == 0) {
    return;
  }

  aabb_t sum_aabb = b;
  vec2_add(sum_aabb.half_size, sum_aabb.half_size, a.half_size);

  hit_t hit = ray_intersect_aabb(a.position, velocity, sum_aabb);
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

  for (u32 i = 0; i < dynlist_size(static_body_list); ++i) {
    static_body_t* static_body = physics_static_body_get(i);
    update_sweep_result(&result, body->aabb, static_body->aabb, i, velocity,
                        body->collision_mask, static_body->collision_layer);
  }

  return result;
}

static hit_t sweep_bodies(body_t* body, vec2 velocity) {
  hit_t result = {.time = 0xBBBB};

  for (u32 i = 0; i < dynlist_size(body_list); ++i) {
    body_t* other = physics_body_get(i);
    if (body == other) {
      continue;
    }
    update_sweep_result(&result, body->aabb, other->aabb, i, velocity,
                        body->collision_mask, other->collision_layer);
  }

  return result;
}

static void sweep_response(body_t* body, vec2 velocity) {
  hit_t hit_static_body = sweep_static_bodies(body, velocity);
  hit_t hit_body = sweep_bodies(body, velocity);

  if (hit_body.is_hit) {
    if (body->on_hit != NULL) {
      body->on_hit(body, physics_body_get(hit_body.other_id), hit_body);
    }
  }

  if (hit_static_body.is_hit) {
    // move the body to the point of collision based on the hit time.
    body->aabb.position[0] = hit_static_body.position[0];
    body->aabb.position[1] = hit_static_body.position[1];

    // after moving, stop the body's velocity in the direction of the collision.
    // this prevents it from trying to move further into the wall.
    if (hit_static_body.normal[0] != 0.0f) {
      body->aabb.position[1] += velocity[1];
      body->velocity[0] = 0.0f;
    }
    if (hit_static_body.normal[1] != 0.0f) {
      body->aabb.position[0] += velocity[0];
      body->velocity[1] = 0.0f;
    }

    // kinematic and normal bodies should both still report static collision
    if (body->on_hit_static != NULL) {
      body->on_hit_static(body,
                          physics_static_body_get(hit_static_body.other_id),
                          hit_static_body);
    }
  } else {
    // no collision was found, continue to move the body in its direction
    vec2_add(body->aabb.position, body->aabb.position, velocity);
  }
}

static void stationary_response(body_t* body) {
  dynlist_each(static_body_list, static_body) {
    if ((body->collision_mask & static_body->collision_layer) == 0) {
      continue;
    }
    // the static bodies should repel any overlapping bodies
    if (physics_aabb_intersect_aabb(static_body->aabb, body->aabb)) {
      vec2 penetration_vector;
      aabb_t diff = aabb_minkowski_difference(static_body->aabb, body->aabb);
      aabb_penetration_vector(penetration_vector, diff);

      // move the position by the penetration_vector
      vec2_add(body->aabb.position, body->aabb.position, penetration_vector);
    }
  }
}

void physics_update(f32 delta_time) {
  dynlist_each(body_list, body) {
    if (!body->is_active) {
      continue;
    }

    // kinematic bodies are normal bodies but do not follow gravity
    if (!body->is_kinematic) {
      body->velocity[0] += body->acceleration[0];
      body->velocity[1] +=
          body->acceleration[1] + (gravity * body->gravity_scale);
      if (terminal_velocity > body->velocity[1]) {
        body->velocity[1] = terminal_velocity;
      }
    }

    // we are only doing narrow phase sweep, not a broad phase sweep, which may
    // be necessary for more performance
    for (u32 i = 0; i < iterations; ++i) {
      vec2 scaled_velocity;
      vec2_scale(scaled_velocity, body->velocity, delta_time * tick_rate);

      sweep_response(body, scaled_velocity);
      stationary_response(body);
      physics_clamp_body(body);
    }
  }
}

void physics_clamp_body(body_t* body) {
  f32 x = body->aabb.position[0];
  f32 y = body->aabb.position[1];
  fv2 render_size = render_get_render_size();

  body->aabb.position[0] = clamp(x, 0, render_size.x);
  body->aabb.position[1] = clamp(y, 0, render_size.y);

  if (!float_eq(body->aabb.position[0], x) ||
      !float_eq(body->aabb.position[1], y)) {
    LOG("CLAMPED body position");
  }
}

size_t physics_body_count(void) { return dynlist_size(body_list); }

body_t* physics_body_get(size_t idx) {
  ASSERT(idx < dynlist_size(body_list));
  return &body_list[idx];
}

size_t physics_body_create(vec2 position, vec2 size, vec2 velocity,
                           f32 gravity_scale, u8 collision_layer,
                           u8 collision_mask, bool is_kinematic,
                           on_hit_func on_hit,
                           on_hit_static_func on_hit_static) {
  size_t list_size = dynlist_size(body_list);
  size_t idx = list_size;
  for (size_t i = 0; i < list_size; ++i) {
    body_t* body = physics_body_get(i);
    if (!body->is_active) {
      idx = i;
      break;
    }
  }

  if (idx == list_size) {
    *dynlist_append(body_list) = (body_t){0};
  }

  body_t* body = physics_body_get(idx);

  *body = (body_t){
      .aabb =
          {
              .position = {position[0], position[1]},
              .half_size = {size[0] * 0.5f, size[1] * 0.5f},
          },
      .velocity = {velocity[0], velocity[1]},
      .gravity_scale = gravity_scale,
      .collision_layer = collision_layer,
      .collision_mask = collision_mask,
      .on_hit = on_hit,
      .on_hit_static = on_hit_static,
      .is_kinematic = is_kinematic,
      .is_active = true,
  };

  return idx;
}

size_t physics_static_body_count(void) {
  return dynlist_size(static_body_list);
}

static_body_t* physics_static_body_get(size_t idx) {
  ASSERT(idx < dynlist_size(static_body_list));
  return &static_body_list[idx];
}

size_t physics_static_body_create(vec2 position, vec2 size,
                                  u8 collision_layer) {
  *dynlist_append(static_body_list) = (static_body_t){
      .aabb = {.position = {position[0], position[1]},
               .half_size = {size[0] * 0.5f, size[1] * 0.5f}},
      .collision_layer = collision_layer,
  };

  return dynlist_size(static_body_list) - 1;
}

bool physics_point_intersect_aabb(vec2 point, aabb_t aabb) {
  vec2 min, max;
  aabb_min_max(min, max, aabb);
  return point[0] >= min[0] && point[0] <= max[0] && point[1] >= min[1] &&
         point[1] <= max[1];
}

bool physics_aabb_intersect_aabb(aabb_t a, aabb_t b) {
  vec2 min, max;
  aabb_min_max(min, max, aabb_minkowski_difference(a, b));

  // check if the origin is within this aabb range
  return (min[0] <= 0 && max[0] >= 0 && min[1] <= 0 && max[1] >= 0);
}

void aabb_min_max(vec2 min, vec2 max, aabb_t aabb) {
  vec2_sub(min, aabb.position, aabb.half_size);
  vec2_add(max, aabb.position, aabb.half_size);
}

aabb_t aabb_minkowski_difference(aabb_t a, aabb_t b) {
  aabb_t result;
  vec2_sub(result.position, a.position, b.position);
  vec2_add(result.half_size, a.half_size, b.half_size);

  return result;
}

void aabb_penetration_vector(vec2 r, aabb_t mink_aabb) {
  // the smallest vector that will separate the boxes
  // diagonals will never be the shortest, so we can check each side
  vec2 min, max;
  aabb_min_max(min, max, mink_aabb);

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

hit_t ray_intersect_aabb(vec2 pos, vec2 magnitude, aabb_t aabb) {
  hit_t hit = {0};
  vec2 min, max;
  aabb_min_max(min, max, aabb);

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
