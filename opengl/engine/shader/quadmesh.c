#include "quadmesh.h"

QuadMesh *quadmesh_create(float w, float h) {
  QuadMesh *quad = (QuadMesh *)malloc(sizeof(QuadMesh));

  float vertices[] = {
      -0.5f, -0.5f, 0.0f, 0.0f, //
      0.5f, -0.5f, 1.0f, 0.0f,  //
      0.5f, 0.5f, 1.0f, 1.0f,   //
                                //
      -0.5f, 0.5f, 0.0f, 1.0f,  //
  };

  unsigned int indices[] = {
      0, 1, 2, //
      2, 3, 0, //
  };

  quad->vertexCount = sizeof(indices) / sizeof(indices[0]);

  int len = sizeof(vertices) / sizeof(vertices[0]);
  quad->vertices = (float *)malloc(len * sizeof(float));
  memcpy(quad->vertices, vertices, len * sizeof(float));

  // Blending
  GLCall(glEnable(GL_BLEND));
  GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

  // Vertex Array Object
  quad->VAO = *vertex_array_create();

  // Vertex Buffer Object
  quad->VBO = *vertex_buffer_create(quad->vertices, len * sizeof(float));

  // Link VAO and VBO
  quad->layout = *vertex_buffer_layout_create();
  vertex_buffer_layout_push_float(&quad->layout, 2);
  vertex_buffer_layout_push_float(&quad->layout, 2);
  vertex_array_add_buffer(&quad->VAO, &quad->VBO, &quad->layout);

  // Vertex Indices Object
  quad->IBO = *index_buffer_create(indices, quad->vertexCount);

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
  GLCall(
      glDrawElements(GL_TRIANGLES, quad->vertexCount, GL_UNSIGNED_INT, NULL));
  GLCall(glBindVertexArray(0));
}
