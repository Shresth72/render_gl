#pragma once
#include "../../utils/utils.h"
#include "quadmesh.h"
#include "texture.h"
#include "uniform_map.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  QuadMesh *quad;
  unsigned int shaderId;

  const char *vertexfilepath;
  const char *fragfilepath;
  const char *texturepath;

  Texture *texture;

  UniformHashMap *uniformMap;

  float r, increment, prevR;
} Shader;

Shader *shader_create_with_texture(const char *vertexPath,
                                   const char *fragmentPath, QuadMesh *quad,
                                   Texture *texture);
Shader *shader_create_with_color(const char *vertexPath,
                                 const char *fragmentPath, QuadMesh *quad);
void shader_destroy(Shader *shader);
void shader_render(Shader *shader);
void shader_bind(Shader *shader);
void shader_unbind(Shader *shader);

int shader_get_uniform_location(Shader *shader, const char *name);
void shader_set_uniform1i(Shader *shader, const char *name, int value);
void shader_set_uniform1f(Shader *shader, const char *name, float value);
void shader_set_uniform4f(Shader *shader, const char *name, float v0, float v1,
                          float v2, float v3);
void shader_set_translation(Shader *shader, float x, float y);

unsigned int util_load_shader(const char *vertexFilepath,
                              const char *fragmentFilepath);
