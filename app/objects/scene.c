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
  if (!scene->lights) {
    free(scene);
    return NULL;
  }

  // Initialize Lights
  LightCreateInfo lightInfos[] = {
      // Red light
      {.color = {1.0f, 0.0f, 0.0f},
       .position = {1.0f, 0.0f, 0.0f},
       .strength = 4.0f},

      // Green light
      {.color = {0.0f, 1.0f, 0.0f},
       .position = {3.0f, 2.0f, 0.0f},
       .strength = 4.0f},

      // Cyan light
      {.color = {0.0f, 1.0f, 1.0f},
       .position = {3.0f, 0.0f, 2.0f},
       .strength = 4.0f},
  };

  for (size_t i = 0; i < scene->numLights; ++i) {
    scene->lights[i] = light_create(&lightInfos[i]);
    if (!scene->lights[i]) {
      // Handle light creation failure
      for (size_t j = 0; j < i; ++j) {
        light_destroy(scene->lights[j]);
      }
      free(scene->lights);
      free(scene);
      return NULL;
    }
  }

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
