#pragma once
#include "quadmesh.h"
#include "shader.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>

#include "../../nanovg/src/nanovg.h"
#include "../../nanovg/src/nanovg_gl.h"

typedef struct {
  unsigned int shader;
  QuadMesh *ourQuad;
  NVGcontext *vg;
} Engine;

NVGcontext *initializeNanoVG(const char *fontPath);
Engine *engine_create(const char *fontPath);
void engine_destroy(Engine *engine);
void engine_create_models(Engine *engine);
void engine_render(Engine *engine, int width, int height, int mousePressed,
                   double mouseX, double mouseY);
void engine_shader_render(Engine *engine);
void engine_text_render(Engine *engine, int width, int height, int mousePressed,
                        double mouseX, double mouseY);
