#pragma once
#include "../../utils/errors.h"
#include "../buffers/index_buffer.h"
#include "../buffers/vertex_array.h"
#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  IndexBuffer IBO;
  unsigned int vertexCount;

  VertexBuffer VBO;
  float *vertices;

  VertexArray VAO;
  VertexBufferLayout layout;
} QuadMesh;

QuadMesh *quadmesh_create(float w, float h);
void quadmesh_destroy(QuadMesh *quad);
void quadmesh_render(QuadMesh *quad);
