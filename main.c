#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>

#define WIN_MARGIN 20.0f

static int winw = 1280, winh = 720;
static int frame_count = 0;

void renderText(const char *text, float x, float y) { printf("%s\n", text); }

int main() {
  if (!glfwInit()) {
    return -1;
  }

  GLFWwindow *window = glfwCreateWindow(winw, winh, "Todo", NULL, NULL);
  if (!window) {
    glfwTerminate();
    printf("error creating window");
    return -1;
  }

  glfwMakeContextCurrent(window);

  // init session with a ticker for the window
  frame_count = 0;

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    // Nvim Colors: 0.117647f, 0.117647f, 0.180392f, 1.0f
    glClearColor(0.1f, 0.1f, 0.1f, 0.1f);

    // begin session
    frame_count++;

    // print something on the window session
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Todo Frame Count: %d", frame_count);

    // end session
    renderText(buffer, 10.0f, 10.0f);

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
