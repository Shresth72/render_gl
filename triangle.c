
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int compileShader(unsigned int type, const char *source) {
  unsigned int id = glCreateShader(type);
  glShaderSource(id, 1, &source, NULL);
  glCompileShader(id);

  int result = 0;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);

  if (result == GL_FALSE) {
    int length = 0;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    char *message = (char *)alloca(length);
    glGetShaderInfoLog(id, length, &length, message);

    printf("Failed to compile %s shader\n",
           (type == GL_VERTEX_SHADER) ? "vertex" : "fragment");
    printf("%s\n", message);

    glDeleteShader(id);
    return 0; // Return 0 to indicate failure
  }

  return id;
}

static unsigned int createShader(const char *vertexShaderSource,
                                 const char *fragmentShaderSource) {
  unsigned int program = glCreateProgram();
  unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
  unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glValidateProgram(program);

  glDeleteShader(vs);
  glDeleteShader(fs);

  return program;
}

int main(void) {
  GLFWwindow *window;

  // Initialize the library
  if (!glfwInit())
    return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create a windowed mode window and its OpenGL context
  window = glfwCreateWindow(640, 480, "Shader", NULL, NULL);
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

  const char *vertexShaderSource = "#version 330 core\n"
                                   "layout (location = 0) in vec4 position;\n"
                                   "void main() {\n"
                                   "    gl_Position = position;\n"
                                   "}\n";
  const char *fragmentShaderSource = "#version 330 core\n"
                                     "layout (location = 0) out vec4 color;\n"
                                     "void main() {\n"
                                     "    color = vec4(1.0, 0.0, 0.0, 1.0);\n"
                                     "}\n";

  unsigned int shader = createShader(vertexShaderSource, fragmentShaderSource);
  glUseProgram(shader);

  float positions[6] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.0f, 0.5f};
  unsigned int buffer;
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

  // Loop until the user closes the window
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
