#include "ink.h"

InkObject *ink_object_create() {
  InkObject *inkObj = (InkObject *)malloc(sizeof(InkObject));
  if (!inkObj)
    return NULL;

  float vertices[16];
  unsigned int indices[6];
  get_vertices16(vertices, indices, 1.0f, 1.0f, 1.0f, 1.0f);

  size_t vertexCount = sizeof(vertices) / sizeof(vertices[0]);
  size_t indexCount = sizeof(indices) / sizeof(indices[0]);

  // Create the QuadMesh
  inkObj->quadMesh =
      quadmesh_create(vertices, indices, vertexCount, indexCount);
  if (!inkObj->quadMesh) {
    free(inkObj);
    return NULL;
  }

  // Create the Texture
  inkObj->texture = texture_create("textures/ink.png");
  if (!inkObj->texture) {
    quadmesh_destroy(inkObj->quadMesh);
    free(inkObj);
    return NULL;
  }

  // Create the Shader
  inkObj->shader = shader_create_with_texture(
      "shaders/ink/vertex.glsl", "shaders/ink/fragment.glsl", inkObj->quadMesh,
      inkObj->texture);
  if (!inkObj->shader) {
    texture_destroy(inkObj->texture);
    quadmesh_destroy(inkObj->quadMesh);
    free(inkObj);
    return NULL;
  }

  return inkObj;
}

void ink_object_destroy(InkObject *inkObj) {
  if (inkObj) {
    shader_destroy(inkObj->shader);
    texture_destroy(inkObj->texture);
    quadmesh_destroy(inkObj->quadMesh);
    free(inkObj);
  }
}

void ink_object_render(InkObject *inkObj) { shader_render(inkObj->shader); }
