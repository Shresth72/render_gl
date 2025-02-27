#pragma once

#include "../../utils/utils.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  unsigned int rendererId;
  unsigned int vertexCount;
} IndexBuffer;

IndexBuffer *index_buffer_create(const unsigned int *data, unsigned int count);
void index_buffer_destroy(IndexBuffer *buffer);
void index_buffer_bind(IndexBuffer *buffer);
void index_buffer_unbind(IndexBuffer *buffer);
