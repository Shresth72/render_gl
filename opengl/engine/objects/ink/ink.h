#pragma once

#include "../../shader/quadmesh.h"
#include "../../shader/shader.h"
#include "../../shader/texture.h"

typedef struct {
  QuadMesh *quadMesh;
  Texture *texture;
  Shader *shader;
} InkObject;

InkObject *ink_object_create();
void ink_object_destroy(InkObject *obj);
void ink_object_render(InkObject *obj);
