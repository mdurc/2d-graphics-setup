#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "../include/glad/gl.h"

#define _exit_failure(_msg)                     \
  fprintf(stderr, "Runtime failure: %s", _msg); \
  return 1;

int main() {
  GLFWwindow* window;

  if (!glfwInit()) {
    _exit_failure("glfwInit");
  }

  window = glfwCreateWindow(640, 480, "window", NULL, NULL);

  if (!window) {
    glfwTerminate();
    _exit_failure("glfwCreateWindow");
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
    _exit_failure("GladLoadFunc");
  }

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f); // set to gray
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
