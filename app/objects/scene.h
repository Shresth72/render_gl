#pragma once
#include "cube.h"
#include "light.h"
#include "player.h"
#include "stdlib.h"
#include "string.h"
#include <cglm/cglm.h>
#include <math.h>

typedef struct {
  Cube *cube;
  Player *player;
  Light **lights;
  size_t numLights;
} Scene;

Scene *scene_create();
void scene_destroy(Scene *scene);
void scene_update(Scene *scene, float rate);
void scene_move_player(Scene *scene, vec3 dPos);
void scene_spin_player(Scene *scene, vec3 dEulers);
void scene_add_light(Scene *scene, Light *light);
