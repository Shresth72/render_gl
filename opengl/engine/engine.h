#pragma once
#include "../utils/errors.h"
#include "objects/ink/ink.h"
#include "render/quadmesh.h"
#include "render/shader.h"
#include "text/text.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdlib.h>

#include "../../nanovg/src/nanovg.h"
#include "../../nanovg/src/nanovg_gl.h"

typedef struct {
  InkObject *inkObj;

  // Shader Objects
  // Shader *tank;

  Text *text;
} Engine;

Engine *engine_create(int width, int height, const char *fontPath);
void engine_destroy(Engine *engine);
void engine_render(Engine *engine, int width, int height, int mousePressed,
                   double mouseX, double mouseY);
