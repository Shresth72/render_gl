#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/src/nanovg.h"
#include "nanovg/src/nanovg_gl.h"

static int winw = 960, winh = 540;
int mouse_pressed = 0;

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

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  winw = width;
  winh = height;
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    mouse_pressed = 1;
  } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    mouse_pressed = 0;
  }
}

int isPointInRect(float px, float py, float x, float y, float w, float h) {
  return px >= x && px <= x + w && py >= y && py <= y + h;
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

  // Setup FrameBuffer Size callback
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // Setup Mouse Button callback
  glfwSetMouseButtonCallback(window, mouse_button_callback);

  printf("%s\n", glGetString(GL_VERSION));

  GLuint vao = 0;
  glCreateVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Initialize NanoVG and Load Font
  const char *fontPath = "/home/shrestha/.fonts/Meditative.ttf";
  NVGcontext *vg = initializeNanoVG(fontPath);
  if (vg == NULL) {
    return -1;
  }

  while (!glfwWindowShouldClose(window)) {
    glViewport(0, 0, winw, winh);

    glClear(GL_COLOR_BUFFER_BIT);

    // Capture Mouse Position
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    //------------- BEGIN VG FRAME ---------------//
    nvgBeginFrame(vg, winw, winh, 1.0f);

    // Draw Text
    nvgFontFace(vg, "sans");
    nvgFontSize(vg, 48.0f);
    nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgText(vg, 10.0f, 8.0f, "Hello, NanoVG!", NULL);

    // Draw Button
    float btnW = 120, btnH = 30;
    float btnX = (winw - btnW) / 2, btnY = (winh - btnH) / 2;
    nvgBeginPath(vg);
    nvgRoundedRect(vg, btnX, btnY, btnW, btnH, 10.f);
    nvgFillColor(vg, nvgRGB(0, 128, 255));
    nvgFill(vg);
    nvgStrokeColor(vg, nvgRGB(0, 0, 0));
    nvgStroke(vg);

    nvgFontSize(vg, 20.0f);
    nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgText(vg, btnX + btnW / 2, btnY + btnH / 2, "Close", NULL);

    if (mouse_pressed &&
        isPointInRect((float)mouseX, (float)mouseY, btnX, btnY, btnW, btnH)) {
      glfwSetWindowShouldClose(window, 1);
    }

    nvgEndFrame(vg);
    //------------- END VG FRAME ---------------//

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  nvgDeleteGL3(vg);
  glfwTerminate();
  return 0;
}
