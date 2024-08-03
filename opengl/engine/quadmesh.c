#include "quadmesh.h"

QuadMesh *quadmesh_create(float w, float h) {
  QuadMesh *quad = (QuadMesh *)malloc(sizeof(QuadMesh));

  float vertices[] = {
      -0.5f * w, -0.5f * h, //
      0.5f * w,  -0.5f * h, //
      0.0f,      0.5f * h,  //
  };

  int len = sizeof(vertices) / sizeof(vertices[0]);
  quad->vertexCount = len / 2;
  quad->vertices = (float *)malloc(len * sizeof(float));
  memcpy(quad->vertices, vertices, len * sizeof(float));

  glGenVertexArrays(1, &quad->VAO);
  glGenBuffers(1, &quad->VBO);

  glBindVertexArray(quad->VAO);

  glBindBuffer(GL_ARRAY_BUFFER, quad->VBO);
  glBufferData(GL_ARRAY_BUFFER, len * sizeof(float), quad->vertices,
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

  glBindVertexArray(0);

  return quad;
}

void quadmesh_destroy(QuadMesh *quad) {
  glDeleteVertexArrays(1, &quad->VAO);
  glDeleteBuffers(1, &quad->VBO);
  free(quad->vertices);
  free(quad);
}

void quadmesh_render(QuadMesh *quad) {
  glBindVertexArray(quad->VAO);
  glDrawArrays(GL_TRIANGLES, 0, quad->vertexCount);
  glBindVertexArray(0);
}
