#include "vertex_buffer.h"

VertexBuffer *vertex_buffer_create(const void *data, unsigned int size) {
  VertexBuffer *vertexBuffer = (VertexBuffer *)malloc(sizeof(VertexBuffer));

  GLCall(glGenBuffers(1, &vertexBuffer->rendererId));
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->rendererId));
  GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));

  return vertexBuffer;
}

void vertex_buffer_destroy(VertexBuffer *buffer) {
  GLCall(glDeleteBuffers(1, &buffer->rendererId));
}

void vertex_buffer_bind(VertexBuffer *buffer) {
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer->rendererId));
}

void vertex_buffer_unbind(VertexBuffer *buffer) {
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
