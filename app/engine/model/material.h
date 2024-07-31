#pragma once
#include "image.h"
#include "stdio.h"
#include "stdlib.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef struct {
  const char *filename;
} MaterialCreateInfo;

typedef struct {
  unsigned int texture;
} Material;

Material *material_create(MaterialCreateInfo *createInfo);
void material_destroy(Material *material);
void material_use(Material *material);
