#include "render.h"

#include "../c-lib/dynlist.h"
#include "render_init.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "../c-lib/misc.h"
#include "../state.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static f32 render_width, render_height;
static f32 render_scale;

static u32 shader_default, shader_batch;
static u32 vao_quad, vbo_quad, ebo_quad;
static u32 vao_line, vbo_line;
static u32 vao_batch, vbo_batch, ebo_batch;
static u32 texture_color;

static DYNLIST(batch_vertex_t) batch_list;

void render_init(u32 width, u32 height, f32 scale) {
  render_init_window(width, height);
  render_scale = scale;
  render_width = width / scale;
  render_height = height / scale;

  render_init_quad(&vao_quad, &vbo_quad, &ebo_quad);
  render_init_batch_quads(&vao_batch, &vbo_batch, &ebo_batch);
  render_init_line(&vao_line, &vbo_line);
  render_init_color_texture(&texture_color);
  render_init_shaders(&shader_default, &shader_batch, render_width,
                      render_height);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  stbi_set_flip_vertically_on_load(1);

  batch_list = dynlist_create(batch_vertex_t, 8);
}

void render_begin(void) {
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  dynlist_clear(batch_list); // clear the list each frame
}

static void render_batch(batch_vertex_t* vertices, size_t num_vertices,
                         u32 texture_id) {
  // update the vbo buffer dynamically
  glBindBuffer(GL_ARRAY_BUFFER, vbo_batch);
  glBufferSubData(GL_ARRAY_BUFFER, 0, num_vertices * sizeof(batch_vertex_t),
                  vertices);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_id);

  glUseProgram(shader_batch);
  glBindVertexArray(vao_batch);
  // the amount of quads we are drawing is count / 4, with six indices per quad
  glDrawElements(GL_TRIANGLES, (num_vertices >> 2) * 6, GL_UNSIGNED_INT, NULL);
}

static void append_quad(vec2 position, vec2 size, vec2 tex_coords, vec4 color) {
  vec4 tex_data = {0, 0, 1, 1}; // default data

  if (tex_coords != NULL) {
    memcpy(tex_data, tex_coords, sizeof(vec4));
  }
  // append the four vertices of the quad into the batch list
  // top left
  *dynlist_append(batch_list) = (batch_vertex_t){
      .position = {position[0], position[1]},
      .tex_coords = {tex_data[0], tex_data[1]},
      .color = {color[0], color[1], color[2], color[3]},
  };

  // top right
  *dynlist_append(batch_list) = (batch_vertex_t){
      .position = {position[0] + size[0], position[1]},
      .tex_coords = {tex_data[2], tex_data[1]},
      .color = {color[0], color[1], color[2], color[3]},
  };

  // bottom right
  *dynlist_append(batch_list) = (batch_vertex_t){
      .position = {position[0] + size[0], position[1] + size[1]},
      .tex_coords = {tex_data[2], tex_data[3]},
      .color = {color[0], color[1], color[2], color[3]},
  };

  // bottom left
  *dynlist_append(batch_list) = (batch_vertex_t){
      .position = {position[0], position[1] + size[1]},
      .tex_coords = {tex_data[0], tex_data[3]},
      .color = {color[0], color[1], color[2], color[3]},
  };
}

void render_end(u32 batch_texture_id) {
  render_batch(batch_list, dynlist_size(batch_list), batch_texture_id);
  glfwSwapBuffers(state.window);
}

void render_quad(vec2 pos, vec2 size, vec4 color) {
  glUseProgram(shader_default);

  mat4x4 model;
  mat4x4_identity(model);
  mat4x4_translate(model, pos[0], pos[1], 0);
  mat4x4_scale_aniso(model, model, size[0], size[1], 1);

  glUniformMatrix4fv(glGetUniformLocation(shader_default, "model"), 1, GL_FALSE,
                     &model[0][0]);
  glUniform4fv(glGetUniformLocation(shader_default, "color"), 1, color);

  // the vao stores all of the vbo's linked to it
  // the vao also stores the glBindBuffer calls when the target is
  // GL_ELEMENT_ARRAY, so when we bind this vao, it will also bind the ebo
  // buffer that we associated to it (ebo_quad)
  glBindVertexArray(vao_quad);
  // glDrawArrays(GL_TRIANGLES, 0, 3); // will draw the vertices from the vbo

  glBindTexture(GL_TEXTURE_2D, texture_color);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // will draw from the ebo

  glBindVertexArray(0);
}

void render_quad_lines(vec2 pos, vec2 size, vec4 color) {
  // using the polygon mode (much more performant)
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  // render_quad(pos, size, color);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  vec2 points[4] = {
      {pos[0] - size[0] * 0.5, pos[1] - size[1] * 0.5},
      {pos[0] + size[0] * 0.5, pos[1] - size[1] * 0.5},
      {pos[0] + size[0] * 0.5, pos[1] + size[1] * 0.5},
      {pos[0] - size[0] * 0.5, pos[1] + size[1] * 0.5},
  };

  render_line_segment(points[0], points[1], color);
  render_line_segment(points[1], points[2], color);
  render_line_segment(points[2], points[3], color);
  render_line_segment(points[3], points[0], color);
}

void render_line_segment(vec2 start, vec2 end, vec4 color) {
  glUseProgram(shader_default);

  // normalize the start to end
  f32 x = end[0] - start[0];
  f32 y = end[1] - start[1];
  f32 line[6] = {0.0f, 0.0f, 0.0f, x, y, 0.0f};

  mat4x4 model;
  mat4x4_identity(model);
  mat4x4_translate(model, start[0], start[1], 0);
  glUniformMatrix4fv(glGetUniformLocation(shader_default, "model"), 1, GL_FALSE,
                     &model[0][0]);
  glUniform4fv(glGetUniformLocation(shader_default, "color"), 1, color);

  glBindTexture(GL_TEXTURE_2D, texture_color);
  glBindVertexArray(vao_line);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_line);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line), line); // update the data
  glDrawArrays(GL_LINES, 0, 2);                            // draw from the vbo

  glBindVertexArray(0);             // unbind vao
  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo
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

void render_sprite_sheet_frame(sprite_sheet_t* sprite_sheet, f32 row,
                               f32 column, vec2 position, vec2 size) {
  vec4 tex_coords;
  calculate_sprite_tex_coords(tex_coords, row, column, sprite_sheet->width,
                              sprite_sheet->height, sprite_sheet->cell_width,
                              sprite_sheet->cell_height);
  if (size == NULL) {
    size = (vec2){sprite_sheet->cell_width, sprite_sheet->cell_height};
  }
  vec2 bottom_left =
      (vec2){position[0] - size[0] * 0.5, position[1] - size[1] * 0.5};
  append_quad(bottom_left, size, tex_coords, WHITE);
}

f32 render_get_scale(void) { return render_scale; }
fv2 render_get_render_size(void) { return (fv2){render_width, render_height}; }

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
