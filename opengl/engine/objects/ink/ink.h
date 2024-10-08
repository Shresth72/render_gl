#pragma once

#include "../../render/quadmesh.h"
#include "../../render/shader.h"
#include "../../render/texture.h"

typedef struct {
  QuadMesh *quadMesh;
  Texture *texture;
  Shader *shader;
} InkObject;

InkObject *ink_object_create();
void ink_object_destroy(InkObject *obj);
void ink_object_render(InkObject *obj);
