#include "render.h"

#include "../c-lib/dynlist.h"
#include "../c-lib/misc.h"
#include "../state.h"
#include "render_init.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static f32 render_width, render_height;
static f32 render_scale;
static vec4 background_color;

// ---- 2d rendering state ----
static u32 texture_slots[8] = {0}; // index zero reserved for default WHITE
static u32 white_texture_id;       // index zero of texture_slots

static u32 shader_2d_default, shader_2d_sprite_batch, shader_2d_line_batch;
static u32 vao_quad, vbo_quad, ebo_quad;
static u32 vao_sprite_batch, vbo_sprite_batch, ebo_sprite_batch;
static u32 vao_line, vbo_line;
static u32 vao_line_batch, vbo_line_batch;
static DYNLIST(batch_sprite_vertex_t) sprite_batch_list;
static DYNLIST(batch_line_vertex_t) line_batch_list;

// ---- 3d rendering state ----
static u32 shader_3d;
static u32 vao_cube, vbo_cube, ebo_cube;

void render_init(u32 width, u32 height, f32 scale, vec4 bg_color) {
  render_init_window(width, height);
  render_scale = scale;
  render_width = width / scale;
  render_height = height / scale;

  if (bg_color == NULL) bg_color = (vec4){0.2f, 0.3f, 0.3f, 1.0f};
  for (u32 i = 0; i < 4; ++i) background_color[i] = bg_color[i];

  // ---- initialize 2d components ----
  render_init_quad(&vao_quad, &vbo_quad, &ebo_quad);
  render_init_line(&vao_line, &vbo_line);
  render_init_color_texture(&texture_slots[0]);
  white_texture_id = texture_slots[0];
  render_init_batch_lines(&vao_line_batch, &vbo_line_batch);
  render_init_batch_texture_quads(&vao_sprite_batch, &vbo_sprite_batch,
                                  &ebo_sprite_batch);

  // ---- initialize 3d components ----
  render_init_cube(&vao_cube, &vbo_cube, &ebo_cube);

  // ---- initialize all shaders ----
  render_init_shaders(&shader_2d_default, &shader_2d_sprite_batch,
                      &shader_2d_line_batch, &shader_3d, render_width,
                      render_height);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  stbi_set_flip_vertically_on_load(1);

  sprite_batch_list = dynlist_create(batch_sprite_vertex_t, 8);
  line_batch_list = dynlist_create(batch_line_vertex_t, 8);

  LOG("Renderer system initialized");
}

void render_destroy(void) {
  dynlist_destroy(sprite_batch_list);
  dynlist_destroy(line_batch_list);
  // TODO destroy all opengl data
  glfwTerminate();
  LOG("Renderer system deinitialized");
}

f32 render_get_render_scale(void) { return render_scale; }
fv2 render_get_render_size(void) { return (fv2){render_width, render_height}; }

void render_begin(void) {
  glClearColor(background_color[0], background_color[1], background_color[2],
               background_color[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  dynlist_clear(sprite_batch_list); // clear the list each frame
  dynlist_clear(line_batch_list);
}

void render_end(void) { glfwSwapBuffers(state.window); }

void render_begin_2d(void) {
  // 2d shaders should already have the orthographic projection matrix set from
  // init, and the specific shaders are used within the individual render funcs.
  glDisable(GL_DEPTH_TEST);
}

void render_sprite_batch(void) {
  size_t num_vertices = dynlist_size(sprite_batch_list);
  if (num_vertices == 0) return;

  // update the vbo buffer dynamically
  glBindBuffer(GL_ARRAY_BUFFER, vbo_sprite_batch);
  glBufferSubData(GL_ARRAY_BUFFER, 0,
                  num_vertices * sizeof(batch_sprite_vertex_t),
                  sprite_batch_list);

  ASSERT(texture_slots[0] == white_texture_id,
         "texture slot 0 should be white_texture_id (%d), but is %d",
         white_texture_id, texture_slots[0]);
  for (u32 i = 0; i < 8; ++i) {
    // fragment shader texture i = our texture id at slot[i]
    glActiveTexture(GL_TEXTURE0 + i);
    u32 id = texture_slots[i];
    /* Use a valid defined texture id to avoid the following warning:
     * UNSUPPORTED (log once): POSSIBLE ISSUE: unit 1 GLD_TEXTURE_INDEX_2D is
     * unloadable and bound to sampler type (Float) - using zero texture because
     * texture unloadable.
     */
    glBindTexture(GL_TEXTURE_2D, id == 0 ? white_texture_id : id);
  }

  glUseProgram(shader_2d_sprite_batch);
  glBindVertexArray(vao_sprite_batch);
  // the amount of quads we are drawing is count / 4, with six indices per quad
  // so we draw all of the required indices that were set up in the batch init
  glDrawElements(GL_TRIANGLES, (num_vertices >> 2) * 6, GL_UNSIGNED_INT, NULL);
}

void render_aabb_line_batch(void) {
  size_t num_vertices = dynlist_size(line_batch_list);
  if (num_vertices == 0) return;

  glBindBuffer(GL_ARRAY_BUFFER, vbo_line_batch);
  glBufferSubData(GL_ARRAY_BUFFER, 0,
                  num_vertices * sizeof(batch_line_vertex_t), line_batch_list);

  glUseProgram(shader_2d_line_batch);
  glBindVertexArray(vao_line_batch);
  glDrawArrays(GL_LINES, 0, num_vertices);
  glBindVertexArray(0);
}

void render_quad(vec2 pos, vec2 size, vec4 color) {
  glUseProgram(shader_2d_default);

  mat4x4 model;
  mat4x4_identity(model);
  mat4x4_translate(model, pos[0], pos[1], 0);
  mat4x4_scale_aniso(model, model, size[0], size[1], 1);

  glUniformMatrix4fv(glGetUniformLocation(shader_2d_default, "model"), 1,
                     GL_FALSE, &model[0][0]);
  glUniform4fv(glGetUniformLocation(shader_2d_default, "color"), 1, color);

  // the vao stores all of the vbo's linked to it
  // the vao also stores the glBindBuffer calls when the target is
  // GL_ELEMENT_ARRAY, so when we bind this vao, it will also bind the ebo
  // buffer that we associated to it (ebo_quad)
  glBindVertexArray(vao_quad);
  glBindTexture(GL_TEXTURE_2D, white_texture_id);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // will draw from the ebo

  glBindVertexArray(0);
}

void render_line_segment(vec2 start, vec2 end, vec4 color) {
  glUseProgram(shader_2d_default);

  // normalize the start to end
  f32 x = end[0] - start[0];
  f32 y = end[1] - start[1];
  f32 line[6] = {0.0f, 0.0f, 0.0f, x, y, 0.0f};

  mat4x4 model;
  mat4x4_identity(model);
  mat4x4_translate(model, start[0], start[1], 0);
  glUniformMatrix4fv(glGetUniformLocation(shader_2d_default, "model"), 1,
                     GL_FALSE, &model[0][0]);
  glUniform4fv(glGetUniformLocation(shader_2d_default, "color"), 1, color);

  glBindTexture(GL_TEXTURE_2D, white_texture_id);
  glBindVertexArray(vao_line);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_line);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line), line); // update the data
  glDrawArrays(GL_LINES, 0, 2);                            // draw from the vbo

  glBindVertexArray(0);             // unbind vao
  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo
}

void render_quad_lines(vec2 pos, vec2 size, vec4 color) {
  // using the polygon mode (much more performant)
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  // render_quad(pos, size, color);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  vec2 top_left = {pos[0] - size[0] * 0.5f, pos[1] - size[1] * 0.5f};
  vec2 top_right = {pos[0] + size[0] * 0.5f, pos[1] - size[1] * 0.5f};
  vec2 bottom_right = {pos[0] + size[0] * 0.5f, pos[1] + size[1] * 0.5f};
  vec2 bottom_left = {pos[0] - size[0] * 0.5f, pos[1] + size[1] * 0.5f};

  *dynlist_append(line_batch_list) =
      (batch_line_vertex_t){.position = {top_left[0], top_left[1]},
                            .color = {color[0], color[1], color[2], color[3]}};
  *dynlist_append(line_batch_list) =
      (batch_line_vertex_t){.position = {top_right[0], top_right[1]},
                            .color = {color[0], color[1], color[2], color[3]}};
  *dynlist_append(line_batch_list) =
      (batch_line_vertex_t){.position = {top_right[0], top_right[1]},
                            .color = {color[0], color[1], color[2], color[3]}};
  *dynlist_append(line_batch_list) =
      (batch_line_vertex_t){.position = {bottom_right[0], bottom_right[1]},
                            .color = {color[0], color[1], color[2], color[3]}};
  *dynlist_append(line_batch_list) =
      (batch_line_vertex_t){.position = {bottom_right[0], bottom_right[1]},
                            .color = {color[0], color[1], color[2], color[3]}};
  *dynlist_append(line_batch_list) =
      (batch_line_vertex_t){.position = {bottom_left[0], bottom_left[1]},
                            .color = {color[0], color[1], color[2], color[3]}};
  *dynlist_append(line_batch_list) =
      (batch_line_vertex_t){.position = {bottom_left[0], bottom_left[1]},
                            .color = {color[0], color[1], color[2], color[3]}};
  *dynlist_append(line_batch_list) =
      (batch_line_vertex_t){.position = {top_left[0], top_left[1]},
                            .color = {color[0], color[1], color[2], color[3]}};
}

void render_aabb(f32* aabb, vec4 color) {
  vec2 size;
  vec2_scale(size, &aabb[2], 2); // scale the halfsize by 2
  render_quad_lines(&aabb[0], size, color);
}

static void calculate_sprite_tex_coords(vec4 result, f32 row, f32 column,
                                        f32 texture_width, f32 texture_height,
                                        f32 cell_width, f32 cell_height) {
  f32 w = 1.0f / (texture_width / cell_width);
  f32 h = 1.0f / (texture_height / cell_height);
  f32 x = column * w;
  // flip the row so that 0,0 index in the texture coordinates is the top left
  f32 y = ((texture_height / cell_height - 1) - row) * h;
  result[0] = x;
  result[1] = y;
  result[2] = x + w;
  result[3] = y + h;
}

static void append_texture_quad(vec2 position, vec2 size, vec2 tex_coords,
                                vec4 color, u32 texture_slot_index) {
  // for batch rendering of the sprite sheet textures/frames
  vec4 tex_data = {0, 0, 1, 1}; // default data

  if (tex_coords != NULL) {
    memcpy(tex_data, tex_coords, sizeof(vec4));
  }
  // append the four vertices of the quad into the batch list
  // top left
  *dynlist_append(sprite_batch_list) = (batch_sprite_vertex_t){
      .position = {position[0], position[1]},
      .tex_coords = {tex_data[0], tex_data[1]},
      .color = {color[0], color[1], color[2], color[3]},
      .texture_slot_index = texture_slot_index,
  };

  // top right
  *dynlist_append(sprite_batch_list) = (batch_sprite_vertex_t){
      .position = {position[0] + size[0], position[1]},
      .tex_coords = {tex_data[2], tex_data[1]},
      .color = {color[0], color[1], color[2], color[3]},
      .texture_slot_index = texture_slot_index,
  };

  // bottom right
  *dynlist_append(sprite_batch_list) = (batch_sprite_vertex_t){
      .position = {position[0] + size[0], position[1] + size[1]},
      .tex_coords = {tex_data[2], tex_data[3]},
      .color = {color[0], color[1], color[2], color[3]},
      .texture_slot_index = texture_slot_index,
  };

  // bottom left
  *dynlist_append(sprite_batch_list) = (batch_sprite_vertex_t){
      .position = {position[0], position[1] + size[1]},
      .tex_coords = {tex_data[0], tex_data[3]},
      .color = {color[0], color[1], color[2], color[3]},
      .texture_slot_index = texture_slot_index,
  };
}

// returns the index of the texture slot associated to texture_id (-1 if full)
static i32 set_texture_slot(u32 texture_id) {
  i32 free_index = -1;
  // the zero'th index is reserved for the default color texture
  for (i32 i = 1; i < 8; ++i) {
    if (texture_slots[i] == texture_id) {
      return i;
    }
    if (texture_slots[i] == 0) {
      free_index = i;
    }
  }

  if (free_index != -1) {
    texture_slots[free_index] = texture_id;
  }

  // returns -1 on failure
  return free_index;
}

void render_sprite_sheet_frame(sprite_sheet_t* sprite_sheet, f32 row,
                               f32 column, vec2 position, vec2 size, vec4 color,
                               bool is_flipped) {
  vec4 tex_coords;
  calculate_sprite_tex_coords(tex_coords, row, column, sprite_sheet->width,
                              sprite_sheet->height, sprite_sheet->cell_width,
                              sprite_sheet->cell_height);
  if (is_flipped) {
    // flip the x-axis
    f32 tmp = tex_coords[0];
    tex_coords[0] = tex_coords[2];
    tex_coords[2] = tmp;
  }

  if (size == NULL) {
    size = (vec2){sprite_sheet->cell_width, sprite_sheet->cell_height};
  }
  vec2 bottom_left =
      (vec2){position[0] - size[0] * 0.5, position[1] - size[1] * 0.5};

  i32 texture_slot = set_texture_slot(sprite_sheet->texture_id);
  ASSERT(texture_slot != -1, "need to implement flushing the texture slots");
  append_texture_quad(bottom_left, size, tex_coords, color, texture_slot);
}

void render_begin_3d(camera_t* camera) {
  glEnable(GL_DEPTH_TEST);

  mat4x4 projection, view;
  // create projection matrix
  mat4x4_perspective(projection, camera->fov_radians, camera->aspect_ratio,
                     camera->near_plane, camera->far_plane);

  // create view matrix
  vec3 center;
  vec3_add(center, camera->position, camera->direction);
  mat4x4_look_at(view, camera->position, center, camera->up);

  // set uniforms
  glUseProgram(shader_3d);
  glUniformMatrix4fv(glGetUniformLocation(shader_3d, "projection"), 1, GL_FALSE,
                     &projection[0][0]);
  glUniformMatrix4fv(glGetUniformLocation(shader_3d, "view"), 1, GL_FALSE,
                     &view[0][0]);
}

void render_cube(mat4x4 model, u32* texture_id) {
  glUseProgram(shader_3d);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_id ? *texture_id : white_texture_id);

  glUniformMatrix4fv(glGetUniformLocation(shader_3d, "model"), 1, GL_FALSE,
                     &model[0][0]);

  glBindVertexArray(vao_cube);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
  glBindVertexArray(0);
}

// for testing triangles
void render_test_setup(u32* out_shader, u32* out_vao, f32 scale) {
  int success;
  char log[512];
  const char* vert_src =
      "#version 330 core\n"
      "layout (location = 0) in vec3 a_pos;\n"
      "layout (location = 1) in vec3 a_color;\n"
      "out vec3 color;\n"
      "void main() {\n"
      "color = a_color;\n"
      "gl_Position = vec4(a_pos, 1.0f);\n"
      "}";
  const char* frag_src =
      "#version 330 core\n"
      "out vec4 frag_color;\n"
      "in vec3 color;\n"
      "void main() {\n"
      "frag_color = vec4(color, 1.0f);\n"
      "}";

  u32 shader_vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(shader_vertex, 1, &vert_src, NULL);
  glCompileShader(shader_vertex);
  glGetShaderiv(shader_vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader_vertex, 512, NULL, log);
    ERROR_EXIT("error compiling vertex shader. %s\n", log);
  }
  u32 shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(shader_fragment, 1, &frag_src, NULL);
  glCompileShader(shader_fragment);
  glGetShaderiv(shader_fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader_fragment, 512, NULL, log);
    ERROR_EXIT("error compiling fragment shader. %s\n", log);
  }

  *out_shader = glCreateProgram();
  glAttachShader(*out_shader, shader_vertex);
  glAttachShader(*out_shader, shader_fragment);
  glLinkProgram(*out_shader);
  glGetProgramiv(*out_shader, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(*out_shader, 512, NULL, log);
    ERROR_EXIT("error linking shader. %s\n", log);
  }
  glUseProgram(*out_shader);
  glDeleteShader(shader_vertex);
  glDeleteShader(shader_fragment);

  f32 vertices[] = {
      // position         // color
      scale * 0.9f, -0.3f, 0.0f, 1.0f, 0.0f, 0.0f, // left
      scale * 0.7f, 0.3f,  0.0f, 0.0f, 1.0f, 0.0f, // top
      scale * 0.5f, -0.3f, 0.0f, 0.0f, 0.0f, 1.0f  // right
  };

  glGenVertexArrays(1, out_vao);
  u32 vbo;
  glGenBuffers(1, &vbo);
  glBindVertexArray(*out_vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void render_test_triangle(u32 shader, u32 vao) {
  glUseProgram(shader);
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
}
