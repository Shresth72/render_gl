#pragma once
#include <cglm/cglm.h>
#include <stdlib.h>

typedef struct {
  vec3 position;
  vec3 color;
  float strength;
} LightCreateInfo;

typedef struct {
  vec3 position;
  vec3 color;
  float strength;
} Light;

Light *light_create(LightCreateInfo *createInfo);
void light_destroy(Light *light);
