#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>

#include "../include/glad/gl.h"

#define _exit_failure(_msg)                     \
  fprintf(stderr, "Runtime failure: %s", _msg); \
  return 1;

const char* vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main() { gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0); }";
const char* fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() { FragColor = vec4(0.8f, 0.3f, 0.02f, 1.0f); }";

int main() {
  GLFWwindow* window;

  if (!glfwInit()) {
    _exit_failure("glfwInit");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // core profile for more modern functions
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window = glfwCreateWindow(800, 800, "window", NULL, NULL);

  if (!window) {
    glfwTerminate();
    _exit_failure("glfwCreateWindow");
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
    _exit_failure("GladLoadFunc");
  }

  // maps normalized coords (NDC, -1 to 1) to frame buffer (pixel) coords
  int fbWidth, fbHeight;
  glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
  glViewport(0, 0, fbWidth, fbHeight);

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  // null is for source string length, but ours are null-terminated
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  // now they are in the program memory so we can clear them
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  GLfloat vertices[] = {
      -0.5f, -0.5f * (float)(sqrt(3)) / 3,    0.0f,
      0.5f,  -0.5f * (float)(sqrt(3)) / 3,    0.0f,
      0.0f,  0.5f * (float)(sqrt(3)) * 2 / 3, 0.0f,
  };

  GLuint VAO, VBO;            // vertex buffer object
  glGenVertexArrays(1, &VAO); // generate VAO before VBO
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // optional
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f); // set to gray
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO); // optional?
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteProgram(shaderProgram);

  glfwTerminate();
  return 0;
}
