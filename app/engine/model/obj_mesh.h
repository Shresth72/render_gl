#pragma once
#include "obj_loader.h"
#include "stdio.h"
#include "stdlib.h"

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
