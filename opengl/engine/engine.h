#pragma once
#include "../utils/errors.h"
#include "quadmesh.h"
#include "shader.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdlib.h>

typedef struct {
  unsigned int shader;
  QuadMesh *ourQuad;

  mat4 projection;
  mat4 view;
  mat4 model;

  unsigned int blockIndex;
  unsigned int blockSize;
  unsigned int UBO;
} Engine;

Engine *engine_create(int width, int height);
void engine_destroy(Engine *engine);
void engine_create_models(Engine *engine);
void engine_render(Engine *engine, int width, int height);
void engine_shader_render(Engine *engine);
