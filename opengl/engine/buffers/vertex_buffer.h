#pragma once

#include "../../utils/utils.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  unsigned int rendererId;
} VertexBuffer;

VertexBuffer *vertex_buffer_create(const void *data, unsigned int size);
void vertex_buffer_destroy(VertexBuffer *buffer);
void vertex_buffer_bind(VertexBuffer *buffer);
void vertex_buffer_unbind(VertexBuffer *buffer);
