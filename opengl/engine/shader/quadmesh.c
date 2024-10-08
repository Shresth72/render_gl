#include "quadmesh.h"

QuadMesh *quadmesh_create(float *vertices, unsigned int *indices,
                          size_t vertexCount, size_t indexCount) {
  QuadMesh *quad = (QuadMesh *)malloc(sizeof(QuadMesh));

  quad->indexCount = indexCount;
  quad->vertices = (float *)malloc(vertexCount * sizeof(float));
  memcpy(quad->vertices, vertices, vertexCount * sizeof(float));

  // Blending
  GLCall(glEnable(GL_BLEND));
  GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

  // Vertex Array Object
  quad->VAO = *vertex_array_create();

  // Vertex Buffer Object
  quad->VBO =
      *vertex_buffer_create(quad->vertices, vertexCount * sizeof(float));

  // Link VAO and VBO
  quad->layout = *vertex_buffer_layout_create();
  vertex_buffer_layout_push_float(&quad->layout, 2);
  vertex_buffer_layout_push_float(&quad->layout, 2);
  vertex_array_add_buffer(&quad->VAO, &quad->VBO, &quad->layout);

  // Vertex Indices Object
  quad->IBO = *index_buffer_create(indices, quad->indexCount);

  return quad;
}

void quadmesh_destroy(QuadMesh *quad) {
  vertex_array_destroy(&quad->VAO);
  vertex_buffer_destroy(&quad->VBO);
  index_buffer_destroy(&quad->IBO);
  vertex_buffer_layout_destroy(&quad->layout);

  free(quad->vertices);
  free(quad);
}

void quadmesh_render(QuadMesh *quad) {
  // Bind VAO and IBO
  vertex_array_bind(&quad->VAO);
  index_buffer_bind(&quad->IBO);

  // Draw Elements
  GLCall(glDrawElements(GL_TRIANGLES, quad->indexCount, GL_UNSIGNED_INT, NULL));
  GLCall(glBindVertexArray(0));
}
