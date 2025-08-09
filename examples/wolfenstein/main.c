#include "engine/c-lib/math.h"
#include "engine/c-lib/misc.h"
#include "engine/config/config.h"
#include "engine/renderer/render.h"
#include "engine/state.h"
#include "engine/time/time.h"

#define MAP_WIDTH 8
#define MAP_HEIGHT 8
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define MOVE_SPEED 5.0f
#define MOUSE_SENSITIVITY 0.005f

state_t state;
static f64 last_mouse_x = SCREEN_WIDTH * 0.5f;
static struct {
  vec2 pos, dir, plane;
} player;
static const u32 world_map[MAP_WIDTH][MAP_HEIGHT] = {
    {1, 1, 1, 1, 1, 1, 1, 1}, {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1}, {1, 0, 0, 1, 1, 0, 0, 1},
    {1, 0, 0, 1, 1, 0, 0, 1}, {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1}, {1, 1, 1, 1, 1, 1, 1, 1},
};

static void input_handle(f32 dt) {
  if (state.input.states[INPUT_KEY_ESCAPE] > 0) {
    glfwSetWindowShouldClose(state.window, true);
  }

  f32 move_step = MOVE_SPEED * dt;

  f32 x = player.pos[0], y = player.pos[1];
  f32 dx = player.dir[0], dy = player.dir[1];
  f32 px = player.plane[0], py = player.plane[1];

  // forward/backward
  if (state.input.states[INPUT_KEY_W]) {
    if (world_map[(u32)(x + dx * move_step)][(u32)y] == 0) {
      player.pos[0] += dx * move_step;
    }
    if (world_map[(u32)x][(u32)(y + dy * move_step)] == 0) {
      player.pos[1] += dy * move_step;
    }
  }
  if (state.input.states[INPUT_KEY_S] > 0) {
    // Move backward if no wall behind
    if (world_map[(u32)(player.pos[0] - player.dir[0] * move_step)]
                 [(u32)player.pos[1]] == 0) {
      player.pos[0] -= player.dir[0] * move_step;
    }
    if (world_map[(u32)player.pos[0]]
                 [(u32)(player.pos[1] - player.dir[1] * move_step)] == 0) {
      player.pos[1] -= player.dir[1] * move_step;
    }
  }

  // strafing left and right using camera plane vec
  if (state.input.states[INPUT_KEY_A]) {
    if (world_map[(u32)(x - px * move_step)][(u32)y] == 0) {
      player.pos[0] -= px * move_step;
    }
    if (world_map[(u32)x][(u32)(y - py * move_step)] == 0) {
      player.pos[1] -= py * move_step;
    }
  }
  if (state.input.states[INPUT_KEY_D]) {
    if (world_map[(u32)(x + px * move_step)][(u32)y] == 0) {
      player.pos[0] += px * move_step;
    }
    if (world_map[(u32)x][(u32)(y + py * move_step)] == 0) {
      player.pos[1] += py * move_step;
    }
  }

  // rotation
  f64 mouse_x;
  glfwGetCursorPos(state.window, &mouse_x, NULL);

  f64 x_offset = mouse_x - last_mouse_x;
  last_mouse_x = mouse_x;
  f32 rot_step = x_offset * MOUSE_SENSITIVITY;

  if (fabs(rot_step) > 0.001f) {
    player.dir[0] = dx * cos(-rot_step) - dy * sin(-rot_step);
    player.dir[1] = dx * sin(-rot_step) + dy * cos(-rot_step);
    player.plane[0] = px * cos(-rot_step) - py * sin(-rot_step);
    player.plane[1] = px * sin(-rot_step) + py * cos(-rot_step);
  }
}

static void render(void) {
  fv2 render_size = render_get_render_size();
  f32 w = render_size.x, h = render_size.y;

  // draw the horizon
  render_quad((vec2){w * 0.5f, h * 0.25f}, (vec2){w, h * 0.5f},
              (vec4){0.3f, 0.3f, 0.3f, 1.0f});
  render_quad((vec2){w * 0.5f, h * 0.75f}, (vec2){w, h * 0.5f},
              (vec4){0.6f, 0.6f, 0.6f, 1.0f});

  // iterate through each pixel and draw a column based on the raycast
  for (i32 x = 0; x < w; ++x) {
    // x coord in space from -1 to 1
    f32 camera_x = 2 * x / w - 1;

    vec2 ray_dir = {player.dir[0] + player.plane[0] * camera_x,
                    player.dir[1] + player.plane[1] * camera_x};

    // cast current position to an index for the world map
    u32 map_x = (u32)player.pos[0];
    u32 map_y = (u32)player.pos[1];

    // length of ray from one x/y side to next x/y side
    vec2 delta_dist = {
        float_eq(ray_dir[0], 0.0f) ? 1e30 : fabs(1.0f / ray_dir[0]),
        float_eq(ray_dir[1], 0.0f) ? 1e30 : fabs(1.0f / ray_dir[1]),
    };

    // length of ray from current position to next x/y side
    vec2 side_dist = {
        (ray_dir[0] < 0 ? (player.pos[0] - map_x)
                        : (map_x + 1.0f - player.pos[0])) *
            delta_dist[0],
        (ray_dir[1] < 0 ? (player.pos[1] - map_y)
                        : (map_y + 1.0f - player.pos[1])) *
            delta_dist[1],
    };

    // direction to step in x or y-direction (+1 or -1)
    i32 step_x = sign(ray_dir[0]), step_y = sign(ray_dir[1]);

    u32 hit = 0; // zero is the non-wall value
    u32 side;

    // dda
    while (hit == 0) {
      if (side_dist[0] < side_dist[1]) {
        side_dist[0] += delta_dist[0];
        map_x += step_x;
        side = 0; // x side wall hit
      } else {
        side_dist[1] += delta_dist[1];
        map_y += step_y;
        side = 1; // y side wall hit
      }
      ASSERT(map_x < MAP_WIDTH && map_y < MAP_HEIGHT);
      // check if ray has hit a wall
      u32 value = world_map[map_x][map_y];
      if (value > 0) hit = value;
    }

    f32 perp_dist;
    if (side == 0) {
      perp_dist = (side_dist[0] - delta_dist[0]);
    } else {
      perp_dist = (side_dist[1] - delta_dist[1]);
    }

    i32 line_height = (i32)(h / perp_dist);

    // lowest and highest pixel to fill in
    f32 draw_start = max(0, -line_height * 0.5f + h * 0.5f);
    f32 draw_end = max(0, line_height * 0.5f + h * 0.5f);

    vec4 color;
    switch (hit) {
      case 1: memcpy(color, RED, sizeof(vec4)); break;
      default: memcpy(color, YELLOW, sizeof(vec4)); break;
    }

    // y sides only
    if (side == 1) {
      vec4_scale(color, color, 0.5f);
    }

    // 1 pixel wide, render height tall, spawned in the mid of screen
    f32 height = draw_end - draw_start;
    render_quad((vec2){x, draw_start + height * 0.5f}, (vec2){1.0f, height},
                color);
  }
}

int main(void) {
  time_init(60);
  config_init(); // for initializing input handling
  render_init(SCREEN_WIDTH, SCREEN_HEIGHT, 1.0f, BLACK);

  player.pos[0] = 6.0f;
  player.pos[1] = 6.0f;
  player.dir[0] = -1.0f;
  player.dir[1] = 0.0f;
  player.plane[0] = 0.0f;
  player.plane[1] = 0.66f;

  // disabling the cursor: we can keep on panning without leaving the window
  glfwSetInputMode(state.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  if (glfwRawMouseMotionSupported()) {
    glfwSetInputMode(state.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  }
  glfwGetCursorPos(state.window, &last_mouse_x, NULL);

  while (!glfwWindowShouldClose(state.window)) {
    time_update();
    input_update();
    input_handle(state.time.delta);

    render_begin();
    render();
    render_end();

    time_update_late();
  }

  render_destroy();
}
