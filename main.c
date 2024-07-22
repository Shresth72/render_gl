#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/src/nanovg.h"
#include "nanovg/src/nanovg_gl.h"
#include "shader_utils.h"

static int winw = 1280, winh = 720;

NVGcontext *initializeNanoVG(const char *fontPath) {
  NVGcontext *vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
  if (vg == NULL) {
    printf("Could not init nanovg.\n");
    return NULL;
  }

  int font = nvgCreateFont(vg, "sans", fontPath);
  if (font == -1) {
    printf("Could not add font.\n");
    nvgDeleteGL3(vg);
    return NULL;
  }

  return vg;
}

void setupBuffer() {
  float positions[6] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.0f, 0.5f};
  unsigned int buffer;
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
}

int main(void) {
  GLFWwindow *window;

  // Initialize the library
  if (!glfwInit())
    return -1;

  // Setup GLFW Version according to supported OPENGL on your system
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create a windowed mode window and its OpenGL context
  window = glfwCreateWindow(winw, winh, "Shader", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);
  if (glewInit() != GLEW_OK) {
    printf("Error initializing GLEW\n");
    return -1;
  }

  printf("%s\n", glGetString(GL_VERSION));

  GLuint vao = 0;
  glCreateVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Load Shaders
  unsigned int shader =
      createShaderFromFiles("vertex_shader.glsl", "fragment_shader.glsl");
  if (shader == 0) {
    return -1;
  }
  glUseProgram(shader);

  // Setup buffer
  setupBuffer();

  // Initialize NanoVG
  const char *fontPath = "/home/shrestha/.fonts/Meditative.ttf";
  NVGcontext *vg = initializeNanoVG(fontPath);
  if (vg == NULL) {
    return -1;
  }

  while (!glfwWindowShouldClose(window)) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glClear(GL_COLOR_BUFFER_BIT);
    // glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    nvgBeginFrame(vg, width, height, 1.0f);

    nvgFontFace(vg, "sans");
    nvgFontSize(vg, 48.0f);
    nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgText(vg, 10.0f, 10.0f, "Hello, NanoVG!", NULL);

    nvgEndFrame(vg);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  nvgDeleteGL3(vg);
  glfwTerminate();
  return 0;
}
