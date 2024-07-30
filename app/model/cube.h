#pragma once
#include "stdio.h"
#include "stdlib.h"
#include <cglm/cglm.h>

typedef struct {
  vec3 position;
  vec3 eulers;
} CubCreateInfo;

typedef struct {
  vec3 position;
  vec3 eulers;
  mat4 modelTransform;
} Cube;

Cube *cube_create(CubCreateInfo *createInfo);
void cube_update(Cube *cube, float rate);
void cube_destroy(Cube *cube);
