#include "vertex_array.h"

VertexArray *vertex_array_create() {
  VertexArray *vao = (VertexArray *)malloc(sizeof(VertexArray));
  glGenVertexArrays(1, &vao->rendererId);
  vertex_array_bind(vao);
  return vao;
}

void vertex_array_destroy(VertexArray *vao) {
  glDeleteVertexArrays(1, &vao->rendererId);
}

void vertex_array_add_buffer(VertexArray *vao, VertexBuffer *vb,
                             VertexBufferLayout *layout) {
  vertex_array_bind(vao);
  vertex_buffer_bind(vb);

  unsigned int offset = 0;
  for (unsigned int i = 0; i < layout->elementCount; i++) {
    VertexBufferElement element = layout->elements[i];
    glEnableVertexAttribArray(i);
    glVertexAttribPointer(i, element.count, element.type, element.normalized,
                          layout->stride, (const void *)(uintptr_t)offset);
    offset += element.count * get_size_of_type(element.type);
  }
}

void vertex_array_bind(VertexArray *vao) { glBindVertexArray(vao->rendererId); }

void vertex_array_unbind(VertexArray *vao) { glBindVertexArray(0); }
