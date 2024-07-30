#include "scene.h"

Scene *scene_create() {
  Scene *scene = (Scene *)malloc(sizeof(Scene));
  if (!scene) {
    return NULL;
  }

  // Initialize Player
  PlayerCreateInfo playerInfo = {
      .position = {0.0f, 0.0f, 1.0f}, //
      .eulers = {0.0f, 90.0f, 0.0f},  //
  };
  scene->player = player_create(&playerInfo);

  // Initialize Cube
  CubCreateInfo cubeInfo = {
      .position = {3.0f, 0.0f, 0.5f}, //
      .eulers = {0.0f, 0.0f, 0.0f},   //
  };
  scene->cube = cube_create(&cubeInfo);

  scene->numLights = 3;
  scene->lights = (Light **)malloc(scene->numLights * sizeof(Light *));

  // Initialize Lights
  LightCreateInfo lightInfo;

  // Red
  lightInfo.color[0] = 1.0f;
  lightInfo.color[1] = 0.0f;
  lightInfo.color[2] = 0.0f;
  lightInfo.position[0] = 1.0f;
  lightInfo.position[1] = 0.0f;
  lightInfo.position[2] = 0.0f;
  lightInfo.strength = 4.0f;
  scene->lights[0] = light_create(&lightInfo);

  // Green
  lightInfo.color[0] = 0.0f;
  lightInfo.color[1] = 1.0f;
  lightInfo.color[2] = 0.0f;
  lightInfo.position[0] = 3.0f;
  lightInfo.position[1] = 2.0f;
  lightInfo.position[2] = 0.0f;
  scene->lights[1] = light_create(&lightInfo);

  // Cyan
  lightInfo.color[0] = 0.0f;
  lightInfo.color[1] = 1.0f;
  lightInfo.color[2] = 1.0f;
  lightInfo.position[0] = 3.0f;
  lightInfo.position[1] = 0.0f;
  lightInfo.position[2] = 2.0f;
  scene->lights[2] = light_create(&lightInfo);

  return scene;
}

void scene_destroy(Scene *scene) {
  if (scene) {
    cube_destroy(scene->cube);
    player_destroy(scene->player);

    for (size_t i = 0; i < scene->numLights; ++i) {
      light_destroy(scene->lights[i]);
    }
    free(scene->lights);

    free(scene);
  }
}

void scene_update(Scene *scene, float rate) {
  player_update(scene->player);
  cube_update(scene->cube, rate);
}

void scene_move_player(Scene *scene, vec3 dPos) {
  glm_vec3_add(scene->player->position, dPos, scene->player->position);
}

void scene_spin_player(Scene *scene, float *dEulers) {
  glm_vec3_add(scene->player->eulers, dEulers, scene->player->eulers);

  if (scene->player->eulers[2] < 0) {
    scene->player->eulers[2] += 360;
  } else if (scene->player->eulers[2] > 360) {
    scene->player->eulers[2] -= 360;
  }

  scene->player->eulers[1] =
      fmaxf(fminf(scene->player->eulers[1], 179.0f), 1.0f);
}
