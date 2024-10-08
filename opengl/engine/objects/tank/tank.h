#pragma once

#include "../../render/quadmesh.h"
#include "../../render/shader.h"
#include "../../render/texture.h"
#include <cglm/cglm.h>

typedef struct {
  QuadMesh *quadMesh;
  Texture *texture;
  Shader *shader;

  float currX, currY; // Tank position
} TankObject;

TankObject *tank_object_create();
void tank_object_destroy(TankObject *obj);
void tank_object_render(TankObject *obj);
void tank_object_move(TankObject *obj, float offsetX, float offsetY);
