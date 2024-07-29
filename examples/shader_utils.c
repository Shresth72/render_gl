#include "shader_utils.h"
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *loadShaderSource(const char *filePath) {
  FILE *file = fopen(filePath, "r");
  if (!file) {
    printf("Failed to open shader file: %s\n", filePath);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *source = (char *)malloc(length + 1);
  if (!source) {
    printf("Failed to allocate memory for shader source\n");
    fclose(file);
    return NULL;
  }

  fread(source, 1, length, file);
  source[length] = '\0';

  fclose(file);
  return source;
}

unsigned int compileShader(unsigned int type, const char *source) {
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

unsigned int createShaderFromFiles(const char *vertexPath,
                                   const char *fragmentPath) {
  char *vertexSource = loadShaderSource(vertexPath);
  char *fragmentSource = loadShaderSource(fragmentPath);

  if (!vertexSource || !fragmentSource) {
    free(vertexSource);
    free(fragmentSource);
    return 0;
  }

  unsigned int program = glCreateProgram();
  unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexSource);
  unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glValidateProgram(program);

  glDeleteShader(vs);
  glDeleteShader(fs);

  free(vertexSource);
  free(fragmentSource);

  return program;
}

unsigned int createShader(const char *vertexShaderSource,
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
