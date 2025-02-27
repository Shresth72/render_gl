#pragma once
#include "../../utils/utils.h"
#include <GL/glew.h>
#include <stdlib.h>

typedef struct {
  unsigned int type;
  unsigned char count;
  unsigned char normalized;
} VertexBufferElement;

typedef struct {
  VertexBufferElement *elements;
  unsigned int elementCount;
  unsigned int elementCapacity;

  unsigned int stride;
} VertexBufferLayout;

VertexBufferLayout *vertex_buffer_layout_create();
void vertex_buffer_layout_destroy(VertexBufferLayout *layout);
void vertex_buffer_layout_push_float(VertexBufferLayout *layout,
                                     unsigned int count);
void vertex_buffer_layout_push_uint(VertexBufferLayout *layout,
                                    unsigned int count);
void vertex_buffer_layout_push_uchar(VertexBufferLayout *layout,
                                     unsigned int count);
unsigned int get_size_of_type(unsigned int type);
