#include "quadmesh.h"
#include <GL/gl.h>

QuadMesh *quadmesh_create(float w, float h) {
  QuadMesh *quad = (QuadMesh *)malloc(sizeof(QuadMesh));

  float vertices[] = {
      -w / 2, -h / 2, 0.0f, 1.0f, //
      w / 2,  -h / 2, 1.0f, 1.0f, //
      w / 2,  h / 2,  1.0f, 0.0f, //
      w / 2,  h / 2,  1.0f, 0.0f, //
      -w / 2, h / 2,  0.0f, 0.0f, //
      -w / 2, -h / 2, 0.0f, 1.0f  //
  };
  int len = sizeof(vertices);

  quad->vertexCount = 6;

  glCreateBuffers(1, &quad->VBO);
  glCreateVertexArrays(1, &quad->VAO);

  glVertexArrayVertexBuffer(quad->VAO, 0, quad->VBO, 0, 4 * sizeof(float));

  glNamedBufferStorage(quad->VBO, len * sizeof(float), vertices,
                       GL_DYNAMIC_STORAGE_BIT);

  glEnableVertexArrayAttrib(quad->VAO, 0);
  glEnableVertexArrayAttrib(quad->VAO, 1);

  glVertexArrayAttribFormat(quad->VAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribFormat(quad->VAO, 1, 2, GL_FLOAT, GL_FALSE,
                            2 * sizeof(float));

  glVertexArrayAttribBinding(quad->VAO, 0, 0);
  glVertexArrayAttribBinding(quad->VAO, 1, 0);

  // glGenBuffers(1, &quad->VBO);
  // glGenVertexArrays(1, &quad->VAO);
  //
  // glBindVertexArray(quad->VAO);
  //
  // glBindBuffer(GL_ARRAY_BUFFER, quad->VBO);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
  //
  // glEnableVertexAttribArray(0);
  // glEnableVertexAttribArray(1);
  //
  // glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void
  // *)0); glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
  //                       (void *)(2 * sizeof(float)));
  //

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
