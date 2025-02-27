#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>

#define ASSERT(x)                                                              \
  if (!(x))                                                                    \
    __builtin_trap();

#define GLCall(x)                                                              \
  gl_clear_error();                                                            \
  x;                                                                           \
  ASSERT(gl_log_call(#x, __FILE__, __LINE__))

// Errors Utils
void gl_clear_error();
int gl_log_call(const char *function, const char *file, int line);

// Object Utils
void get_vertices16(float *vertexArray, unsigned int *indices, float width,
                    float height, float texWidth, float texHeight);
