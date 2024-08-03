#include "utils.h"

void check_gl_error(const char *label) {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    printf("OpenGL Error @ %s: %d\n", label, error);
  }
}
