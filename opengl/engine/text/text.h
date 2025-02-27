#pragma once
#include "../../utils/utils.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>

#include "../../../nanovg/src/nanovg.h"
#include "../../../nanovg/src/nanovg_gl.h"

typedef struct {
  NVGcontext *vg;
} Text;

Text *text_create(const char *fontPath);
void text_destroy(Text *text);
void text_content_render(Text *text, const char *content, float font_size,
                         float x, float y);
void text_render(Text *text, int width, int height, int mousePressed,
                 double mouseX, double mouseY);

int isPointInRect(float px, float py, float x, float y, float w, float h);
void draw_button(Text *text, float x, float y, float w, float h,
                 const char *label, int mousePressed, double mouseX,
                 double mouseY, void (*action)());
void close_action();
void minimize_action();
void maximize_action();
