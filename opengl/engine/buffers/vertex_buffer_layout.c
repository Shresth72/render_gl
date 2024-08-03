#include "vertex_buffer_layout.h"

VertexBufferLayout *vertex_buffer_layout_create() {
  VertexBufferLayout *layout =
      (VertexBufferLayout *)malloc(sizeof(VertexBufferLayout));
  layout->elements = NULL;
  layout->stride = 0;
  layout->elementCount = 0;
  layout->elementCapacity = 0;
  return layout;
}

void vertex_buffer_layout_destroy(VertexBufferLayout *layout) {
  if (layout->elements) {
    free(layout->elements);
  }
}

void vertex_buffer_layout_reserve(VertexBufferLayout *layout,
                                  unsigned int new_capacity) {
  layout->elements = (VertexBufferElement *)realloc(
      layout->elements, new_capacity * sizeof(VertexBufferElement));
  layout->elementCapacity = new_capacity;
}

void vertex_buffer_layout_push_float(VertexBufferLayout *layout,
                                     unsigned int count) {
  if (layout->elementCount >= layout->elementCapacity) {
    vertex_buffer_layout_reserve(
        layout, layout->elementCapacity == 0 ? 4 : layout->elementCapacity * 2);
  }
  layout->elements[layout->elementCount++] =
      (VertexBufferElement){GL_FLOAT, count, GL_FALSE};
  layout->stride += get_size_of_type(GL_FLOAT) * count;
}

void vertex_buffer_layout_push_uint(VertexBufferLayout *layout,
                                    unsigned int count) {
  if (layout->elementCount >= layout->elementCapacity) {
    vertex_buffer_layout_reserve(
        layout, layout->elementCapacity == 0 ? 4 : layout->elementCapacity * 2);
  }
  layout->elements[layout->elementCount++] =
      (VertexBufferElement){GL_UNSIGNED_INT, count, GL_FALSE};
  layout->stride += get_size_of_type(GL_UNSIGNED_INT) * count;
}

void vertex_buffer_layout_push_uchar(VertexBufferLayout *layout,
                                     unsigned int count) {
  if (layout->elementCount >= layout->elementCapacity) {
    vertex_buffer_layout_reserve(
        layout, layout->elementCapacity == 0 ? 4 : layout->elementCapacity * 2);
  }
  layout->elements[layout->elementCount++] =
      (VertexBufferElement){GL_UNSIGNED_BYTE, count, GL_TRUE};
  layout->stride += get_size_of_type(GL_UNSIGNED_BYTE) * count;
}

unsigned int get_size_of_type(unsigned int type) {
  switch (type) {
  case GL_FLOAT:
    return sizeof(GLfloat);
  case GL_UNSIGNED_INT:
    return sizeof(GLuint);
  case GL_UNSIGNED_BYTE:
    return sizeof(GLubyte);
  default:
    return 0;
  }
}
