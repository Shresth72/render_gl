#include "utils.h"

void gl_clear_error() {
  while (glGetError() != GL_NO_ERROR)
    ;
}

int gl_log_call(const char *function, const char *file, int line) {
  GLenum error;
  while ((error = glGetError()) != GL_NO_ERROR) {
    printf("[OpenGL Error] (%u): line %d: %s: %s\n", error, line, function,
           file);
    return 0;
  }
  return 1;
}
