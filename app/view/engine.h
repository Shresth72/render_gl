#pragma once
#include "quadmesh.h"
#include "shader.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>

typedef struct {
  unsigned int shader;
  QuadMesh *ourQuad;
} Engine;

Engine *engine_create(int width, int height);
void engine_destroy(Engine *engine);
void engine_create_models(Engine *engine);
void engine_render(Engine *engine);
