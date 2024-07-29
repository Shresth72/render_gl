#include "quadmesh.h"

QuadMesh *quadmesh_create(float w, float h) {
  QuadMesh *quad = (QuadMesh *)malloc(sizeof(QuadMesh));

  float positions[] = {
      -w / 2, -h / 2, //
      w / 2,  -h / 2, //
      0.0f,   h / 2,  //
  };

  quad->vertexCount = 3;
  quad->vertices = (float *)malloc(sizeof(positions));
  memcpy(quad->vertices, positions, sizeof(positions));

  glGenVertexArrays(1, &quad->VAO);
  glGenBuffers(1, &quad->VBO);
  glBindVertexArray(quad->VAO);

  glBindBuffer(GL_ARRAY_BUFFER, quad->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

  return quad;
}

void quadmesh_destroy(QuadMesh *quad) {
  glDeleteVertexArrays(1, &quad->VAO);
  glDeleteBuffers(1, &quad->VBO);
  free(quad->vertices);
  free(quad);
}
