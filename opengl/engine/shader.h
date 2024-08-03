#pragma once
#include "../utils/errors.h"
#include "quadmesh.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  unsigned int shader;
  int location;
  float r, increment;
  QuadMesh *ourQuad;
} ShaderEngine;

ShaderEngine *shader_engine_create(const char *vertexPath,
                                   const char *fragmentPath);
void shader_engine_destroy(ShaderEngine *shaderEngine);
void shader_engine_render(ShaderEngine *shaderEngine);

unsigned int util_load_shader(const char *vertexFilepath,
                              const char *fragmentFilepath);
