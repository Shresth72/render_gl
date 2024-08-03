#include "index_buffer.h"

IndexBuffer *index_buffer_create(const unsigned int *data, unsigned int count) {
  IndexBuffer *indexBuffer = (IndexBuffer *)malloc(sizeof(IndexBuffer));
  indexBuffer->vertexCount = count;

  GLCall(glGenBuffers(1, &indexBuffer->rendererId));
  GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->rendererId));
  GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), data,
                      GL_STATIC_DRAW));

  return indexBuffer;
}

void index_buffer_destroy(IndexBuffer *buffer) {
  GLCall(glDeleteBuffers(1, &buffer->rendererId));
}

void index_buffer_bind(IndexBuffer *buffer) {
  GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->rendererId));
}

void index_buffer_unbind(IndexBuffer *buffer) {
  GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
