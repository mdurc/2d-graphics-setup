#include <math.h>

#include "engine/c-lib/misc.h"
#include "engine/font/font.h"
#include "engine/renderer/render.h"
#include "engine/renderer/render_init.h"
#include "engine/state.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

state_t state;

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define MAP_WIDTH 10
#define MAP_HEIGHT 10
#define RAD(deg) ((deg) * M_PI / 180.0f)

const u32 world_map[MAP_WIDTH * MAP_HEIGHT] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1,
    1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1,
    0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static f64 last_mouse_x = 400, last_mouse_y = 300;
static f32 yaw = 90.0f;
static f32 pitch = 0.0f;
static bool first_mouse = true;

static camera_t camera = {
    .position = {3.5f, 0.5f, 2.0f},
    .direction = {0.0f, 0.0f, -1.0f},
    .up = {0.0f, 1.0f, 0.0f},
    .fov_radians = RAD(45.0f),
    .aspect_ratio = (f32)SCREEN_WIDTH / (f32)SCREEN_HEIGHT,
    .near_plane = 0.1f,
    .far_plane = 100.0f,
};

static void update_camera_direction(void) {
  vec3 front;
  front[0] = cos(RAD(yaw)) * cos(RAD(pitch));
  front[1] = sin(RAD(pitch));
  front[2] = sin(RAD(yaw)) * cos(RAD(pitch));
  vec3_norm(camera.direction, front);
}

static void input_handle(f32 dt) {
  if (state.input.states[INPUT_KEY_ESCAPE]) {
    glfwSetWindowShouldClose(state.window, true);
  }

  f64 mouse_x, mouse_y;
  glfwGetCursorPos(state.window, &mouse_x, &mouse_y);

  if (first_mouse) {
    last_mouse_x = mouse_x;
    last_mouse_y = mouse_y;
    first_mouse = false;
  }

  f32 x_offset = mouse_x - last_mouse_x;
  f32 y_offset = last_mouse_y - mouse_y;
  last_mouse_x = mouse_x;
  last_mouse_y = mouse_y;

  f32 sensitivity = 0.1f;
  x_offset *= sensitivity;
  y_offset *= sensitivity;

  yaw += x_offset;
  pitch += y_offset;

  if (pitch > 89.0f) pitch = 89.0f;
  if (pitch < -89.0f) pitch = -89.0f;

  update_camera_direction();

  f32 camera_speed = 2.5f * dt;
  vec3 right;
  vec3_mul_cross(right, camera.direction, camera.up);
  vec3_norm(right, right);

  vec3 move_vector_xz = {0};
  if (state.input.states[INPUT_KEY_W]) {
    vec3_add(move_vector_xz, move_vector_xz,
             (vec3){camera.direction[0], 0, camera.direction[2]});
  }
  if (state.input.states[INPUT_KEY_S]) {
    vec3_sub(move_vector_xz, move_vector_xz,
             (vec3){camera.direction[0], 0, camera.direction[2]});
  }
  if (state.input.states[INPUT_KEY_A]) {
    vec3_sub(move_vector_xz, move_vector_xz, right);
  }
  if (state.input.states[INPUT_KEY_D]) {
    vec3_add(move_vector_xz, move_vector_xz, right);
  }

  // normalize and scale horizontal movement
  if (vec3_len(move_vector_xz) > 0) {
    vec3_norm(move_vector_xz, move_vector_xz);
    vec3_scale(move_vector_xz, move_vector_xz, camera_speed);
  }

  // vertical movement
  if (state.input.states[INPUT_KEY_UP]) {
    camera.position[1] += camera_speed;
  }
  if (state.input.states[INPUT_KEY_DOWN]) {
    camera.position[1] -= camera_speed;
  }

  if (state.input.states[INPUT_KEY_ESCAPE]) {
    glfwSetWindowShouldClose(state.window, true);
  }

  // basic collision detection
  bool is_colliding_with_walls =
      (camera.position[1] > 0.0f && camera.position[1] < 1.0f);

  int next_map_x = (int)(camera.position[0] + move_vector_xz[0]);
  int next_map_z = (int)(camera.position[2] + move_vector_xz[2]);

  // if we are at wall height and the next tile is a wall, prevent movement.
  if (is_colliding_with_walls &&
      world_map[next_map_z * MAP_WIDTH + next_map_x] != 0) {
  } else {
    vec3_add(camera.position, camera.position, move_vector_xz);
  }
}

static void render_3d(sprite_sheet_t* bg_sheet) {
  render_begin_3d(&camera);
  {
    mat4x4 floor_model;
    mat4x4_identity(floor_model);
    mat4x4_translate(floor_model, MAP_WIDTH / 2.0f, 0.0f, MAP_HEIGHT / 2.0f);
    mat4x4_scale_aniso(floor_model, floor_model, MAP_WIDTH, 0.01f, MAP_HEIGHT);
    render_cube(floor_model, &bg_sheet->texture_id);

    for (u32 y = 0; y < MAP_HEIGHT; ++y) {
      for (u32 x = 0; x < MAP_WIDTH; ++x) {
        if (world_map[y * MAP_WIDTH + x] == 1) {
          mat4x4 model;
          mat4x4_identity(model);
          mat4x4_translate(model, (f32)x + 0.5f, 0.5f, (f32)y + 0.5f);
          render_cube(model, &bg_sheet->texture_id);
        }
      }
    }
  }
}

static void render_2d(sprite_sheet_t* font_sheet) {
  render_begin_2d();
  {
    fv2 render_size = render_get_render_size();
    render_quad_lines((vec2){render_size.x / 2.0f, render_size.y / 2.0f},
                      (vec2){10, 10}, (vec4){1, 1, 1, 0.8f});
    font_render_str(font_sheet,
                    "WASD for movement\nUP arrow to rise\nDOWN arrow to "
                    "descend\nMouse to pan",
                    (vec2){30, SCREEN_HEIGHT - 30}, (vec2){25, 25}, BLACK);
  }
  render_aabb_line_batch();
  render_sprite_batch();
}

int main(void) {
  time_init(60);
  config_init(); // for initializing input handling
  render_init(SCREEN_WIDTH, SCREEN_HEIGHT, 1.0f,
              (vec4){0.5f, 0.8f, 1.0f, 1.0f});

  glfwSetInputMode(state.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  if (glfwRawMouseMotionSupported()) {
    glfwSetInputMode(state.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  }

  sprite_sheet_t font_sheet;
  render_init_sprite_sheet(&font_sheet, "res/sample_font.png", 8, 8);
  sprite_sheet_t bg_sheet;
  render_init_sprite_sheet(&bg_sheet, "res/sample_bg.png", 8, 8);

  update_camera_direction();

  while (!glfwWindowShouldClose(state.window)) {
    time_update();
    input_update();
    input_handle(state.time.delta);

    render_begin();
    render_3d(&bg_sheet);
    render_2d(&font_sheet);
    render_end();
    time_update_late();
  }

  render_destroy();
  return 0;
}
