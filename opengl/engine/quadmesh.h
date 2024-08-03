#pragma once
#include "../utils/errors.h"
#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  unsigned int VBO, VAO, IBO;
  unsigned int vertexCount;
  float *vertices;
} QuadMesh;

QuadMesh *quadmesh_create(float w, float h);
void quadmesh_destroy(QuadMesh *quad);
void quadmesh_render(QuadMesh *quad);
