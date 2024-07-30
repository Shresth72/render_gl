#pragma once
#include "stdio.h"
#include "stdlib.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

typedef struct {
  vec3 size;
} RectangleModelCreateInfo;

typedef struct {
  unsigned int VBO;
  unsigned int VAO;
  size_t vertexCount;
  float *vertices;
} RectangleModel;

RectangleModel *rectangle_model_create(RectangleModelCreateInfo *createInfo);
void rectangle_model_destroy(RectangleModel *model);
