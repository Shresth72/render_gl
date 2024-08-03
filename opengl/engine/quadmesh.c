#include "quadmesh.h"

QuadMesh *quadmesh_create(float w, float h) {
  QuadMesh *quad = (QuadMesh *)malloc(sizeof(QuadMesh));

  float vertices[] = {
      -0.5f, -0.5f, //
      0.5f, -0.5f,  //
      0.5f, 0.5f,   //
                    //
      -0.5f, 0.5f,  //
  };

  unsigned int indices[] = {
      0, 1, 2, //
      2, 3, 0, //
  };

  quad->vertexCount = sizeof(indices) / sizeof(indices[0]);

  int len = sizeof(vertices) / sizeof(vertices[0]);
  quad->vertices = (float *)malloc(len * sizeof(float));
  memcpy(quad->vertices, vertices, len * sizeof(float));

  // Vertex Buffer and Vertex Attributes Object
  GLCall(glGenVertexArrays(1, &quad->VAO));
  GLCall(glGenBuffers(1, &quad->VBO));

  GLCall(glBindVertexArray(quad->VAO));

  GLCall(glBindBuffer(GL_ARRAY_BUFFER, quad->VBO));
  GLCall(glBufferData(GL_ARRAY_BUFFER, quad->vertexCount * 2 * sizeof(float),
                      quad->vertices, GL_STATIC_DRAW));

  GLCall(glEnableVertexAttribArray(0));
  GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                               (void *)0));

  // Vertex Indices Object
  GLCall(glGenBuffers(1, &quad->IBO));
  GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad->IBO));
  GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                      quad->vertexCount * sizeof(unsigned int), indices,
                      GL_STATIC_DRAW));

  GLCall(glBindVertexArray(0));

  return quad;
}

void quadmesh_destroy(QuadMesh *quad) {
  GLCall(glDeleteVertexArrays(1, &quad->VAO));
  GLCall(glDeleteBuffers(1, &quad->VBO));
  free(quad->vertices);
  free(quad);
}

void quadmesh_render(QuadMesh *quad) {
  GLCall(glBindVertexArray(quad->VAO));
  GLCall(
      glDrawElements(GL_TRIANGLES, quad->vertexCount, GL_UNSIGNED_INT, NULL));
  GLCall(glBindVertexArray(0));
}
