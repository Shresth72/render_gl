#pragma once

#include "../../physics/aabb.h"
#include "../../render/quadmesh.h"
#include "../../render/shader.h"
#include "../../render/texture.h"
#include <cglm/cglm.h>

typedef struct {
  QuadMesh *quadMesh;
  Texture *texture;
  Shader *shader;

  float currX, currY; // Tank position

  float jumpVelocity;
  int isJumping;
  float gravity;
  float groundLevel;

  AABB boundingBox;
} TankObject;

TankObject *tank_object_create();
void initialize_tank_defaults(TankObject *tankObj);

void tank_object_destroy(TankObject *obj);
void tank_object_render(TankObject *obj);
void tank_object_move(TankObject *obj, float offsetX, float offsetY,
                      int spacePressed);

float calculate_quad_width(float *vertices);
float calculate_quad_height(float *vertices);
