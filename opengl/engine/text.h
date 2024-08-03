#pragma once
#include "../utils/errors.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>

#include "../../nanovg/src/nanovg.h"
#include "../../nanovg/src/nanovg_gl.h"

typedef struct {
  NVGcontext *vg;
} TextEngine;

TextEngine *text_engine_create(const char *fontPath);
void text_engine_destroy(TextEngine *textEngine);
void text_engine_render(TextEngine *textEngine, int width, int height,
                        int mousePressed, double mouseX, double mouseY);
