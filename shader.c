#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "shader_utils.h"

static int winw = 960, winh = 540;

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  winw = width;
  winh = height;
}

// Don't really understand this right now
void setupBuffer() {
  float positions[6] = {
      -0.5f, -0.5f, //
      0.5f,  -0.5f, //
      0.0f,  0.5f   //
  };
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

  // Setup FrameBuffer Size callback
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  printf("%s\n", glGetString(GL_VERSION));

  GLuint vao = 0;
  glCreateVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Load Shaders
  unsigned int shader = createShaderFromFiles("shader_source_vertex.glsl",
                                              "shader_source_fragment.glsl");
  if (shader == 0) {
    return -1;
  }
  glUseProgram(shader);

  // Setup buffer
  setupBuffer();

  while (!glfwWindowShouldClose(window)) {
    // Render here
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
