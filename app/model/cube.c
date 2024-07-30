#include "cube.h"

Cube *cube_create(CubCreateInfo *createInfo) {
  Cube *cube = (Cube *)malloc(sizeof(Cube));
  if (!cube) {
    return NULL;
  }

  glm_vec3_copy(createInfo->position, cube->position);
  glm_vec3_copy(createInfo->eulers, cube->eulers);
  glm_mat4_identity(cube->modelTransform);

  return cube;
}

void cube_update(Cube *cube, float rate) {
  // Update euler angles
  cube->eulers[0] += 0.001f * rate;
  if (cube->eulers[0] > 360.0f) {
    cube->eulers[0] -= 360.0f;
  }

  cube->eulers[1] += 0.002f * rate;
  if (cube->eulers[1] > 360.0f) {
    cube->eulers[1] -= 360.0f;
  }

  // Update model transform
  glm_mat4_identity(cube->modelTransform);
  glm_translate(cube->modelTransform, cube->position);

  glm_rotate(cube->modelTransform, glm_rad(cube->eulers[0]),
             (vec3){1.0f, 0.0f, 0.0f}); // Rotate around x-axis
  glm_rotate(cube->modelTransform, glm_rad(cube->eulers[1]),
             (vec3){0.0f, 1.0f, 0.0f}); // Rotate around y-axis
  glm_rotate(cube->modelTransform, glm_rad(cube->eulers[2]),
             (vec3){0.0f, 0.0f, 1.0f}); // Rotate around z-axis
}

void cube_destroy(Cube *cube) { free(cube); }
