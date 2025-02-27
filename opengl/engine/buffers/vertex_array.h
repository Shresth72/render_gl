#pragma once

#include "../../utils/utils.h"
#include "vertex_buffer.h"
#include "vertex_buffer_layout.h"
#include <GL/glew.h>
#include <stdlib.h>

typedef struct {
  unsigned int rendererId;
} VertexArray;

VertexArray *vertex_array_create();
void vertex_array_destroy(VertexArray *vao);
void vertex_array_add_buffer(VertexArray *vao, VertexBuffer *vb,
                             VertexBufferLayout *layout);
void vertex_array_bind(VertexArray *vao);
void vertex_array_unbind(VertexArray *vao);
