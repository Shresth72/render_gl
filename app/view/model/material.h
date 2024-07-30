#pragma once
#include "image.h"

typedef struct {
  const char *filename;
} MaterialCreateInfo;

typedef struct {
  unsigned int texture;
} Material;

Material *material_create(MaterialCreateInfo *createInfo);
void material_destroy(Material *material);
void material_use(Material *material);
