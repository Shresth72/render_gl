#pragma once
#include "../../utils/errors.h"
#include "../quadmesh.h"
#include "uniform_map.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  QuadMesh *ourQuad;
  unsigned int shaderId;

  const char *vertexfilepath;
  const char *fragfilepath;

  UniformHashMap *uniformMap;

  float r, increment;
} Shader;

Shader *shader_create(const char *vertexPath, const char *fragmentPath);
void shader_destroy(Shader *shader);
void shader_render(Shader *shader);
void shader_bind(Shader *shader);
void shader_unbind(Shader *shader);
void shader_set_uniform4f(Shader *shader, const char *name, float v0, float v1,
                          float v2, float v3);
int shader_get_uniform_location(Shader *shader, const char *name);

unsigned int util_load_shader(const char *vertexFilepath,
                              const char *fragmentFilepath);
