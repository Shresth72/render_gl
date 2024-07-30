#pragma once
#include <cglm/cglm.h>
#include <math.h>

typedef struct {
  vec3 position;
  vec3 eulers;
} PlayerCreateInfo;

typedef struct {
  vec3 position;
  vec3 eulers;
  vec3 up;
  vec3 forwards;
  vec3 right;

  mat4 viewTransform;
} Player;

Player *player_create(PlayerCreateInfo *createInfo);
void player_update(Player *player);
void player_destroy(Player *player);
