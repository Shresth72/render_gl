#include "light.h"

Light *light_create(LightCreateInfo *createInfo) {
  Light *light = (Light *)malloc(sizeof(Light));
  if (!light) {
    return NULL;
  }

  glm_vec3_copy(createInfo->position, light->position);
  glm_vec3_copy(createInfo->color, light->color);
  light->strength = createInfo->strength;

  return light;
}

void light_destroy(Light *light) { free(light); }
