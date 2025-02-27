#pragma once
#include "../../utils/utils.h"
#include "../buffers/index_buffer.h"
#include "../buffers/vertex_array.h"
#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  IndexBuffer IBO;
  unsigned int indexCount;

  VertexBuffer VBO;
  float *vertices;
  unsigned int *indices;

  VertexArray VAO;
  VertexBufferLayout layout;
} QuadMesh;

QuadMesh *quadmesh_create(float *vertices, unsigned int *indices,
                          size_t vertexCount, size_t indexCount);
void quadmesh_destroy(QuadMesh *quad);
void quadmesh_render(QuadMesh *quad);
