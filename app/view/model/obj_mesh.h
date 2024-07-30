#pragma once
#include "obj_loader.h"

typedef struct {
  const char *filename;
  mat4 preTransform;
} MeshCreateInfo;

typedef struct {
  unsigned int VBO;
  unsigned int VAO;
  unsigned int vertexCount;
} ObjMesh;

ObjMesh *obj_mesh_create(MeshCreateInfo *createInfo);
void obj_mesh_destroy(ObjMesh *mesh);
