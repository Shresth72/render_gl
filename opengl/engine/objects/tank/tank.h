#pragma once
#include "../../../utils/errors.h"
#include "../../buffers/index_buffer.h"
#include "../../buffers/vertex_array.h"
#include "../../render/texture.h"
#include "../../render/uniform_map.h"
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

  int currX, currY;
} TankMesh;

TankMesh *tankmesh_create(float w, float h);
void tankmesh_destroy(TankMesh *quad);
void tankmesh_render(TankMesh *quad);
