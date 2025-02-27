#define NANOVG_GL3_IMPLEMENTATION
#include "text.h"

Text *text_create(const char *fontPath) {
  Text *text = (Text *)malloc(sizeof(Text));
  text->vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);

  if (text->vg == NULL) {
    free(text);
    return NULL;
  }

  int font = nvgCreateFont(text->vg, "sans", fontPath);
  if (font == -1) {
    printf("Could not add font.\n");
    nvgDeleteGL3(text->vg);
    free(text);
    return NULL;
  }

  return text;
}

void text_destroy(Text *text) {
  nvgDeleteGL3(text->vg);
  free(text);
}

void text_content_render(Text *text, const char *content, float font_size,
                         float x, float y) {
  nvgFontFace(text->vg, "sans");
  nvgFontSize(text->vg, font_size);
  nvgFillColor(text->vg, nvgRGBA(255, 255, 255, 255));
  nvgTextAlign(text->vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
  nvgText(text->vg, x, y, content, NULL);
}

// Render
void text_render(Text *text, int width, int height, int mousePressed,
                 double mouseX, double mouseY) {
  nvgBeginFrame(text->vg, width, height, 1.0f);

  // Draw Text
  text_content_render(text, "Hello Srijan!", 48.0f, 10.0f, 8.0f);

  // Draw Buttons
  float buttonWidth = 40.0f;
  float buttonHeight = 30.0f;
  float buttonY = 10.0f; // No padding at the top

  // Draw Minimize Button
  draw_button(text, width - 2 * buttonWidth, buttonY - 10.0f, buttonWidth,
              buttonHeight, "_", mousePressed, mouseX, mouseY, minimize_action);
  // Draw Close Button
  draw_button(text, width - 1 * buttonWidth, buttonY - 10.0f, buttonWidth,
              buttonHeight, "X", mousePressed, mouseX, mouseY, close_action);

  nvgEndFrame(text->vg);
}

// UTILS
int isPointInRect(float px, float py, float x, float y, float w, float h) {
  return px >= x && px <= x + w && py >= y && py <= y + h;
}

void close_action() { glfwSetWindowShouldClose(glfwGetCurrentContext(), 1); }

void minimize_action() { glfwIconifyWindow(glfwGetCurrentContext()); }

void maximize_action() { glfwMaximizeWindow(glfwGetCurrentContext()); }

void draw_button(Text *text, float x, float y, float w, float h,
                 const char *label, int mousePressed, double mouseX,
                 double mouseY, void (*action)()) {
  nvgBeginPath(text->vg);
  nvgRect(text->vg, x, y, w, h);
  nvgFillColor(text->vg, nvgRGB(200, 0, 0));
  nvgFill(text->vg);
  nvgStrokeColor(text->vg, nvgRGB(0, 0, 0));
  nvgStroke(text->vg);

  nvgFontSize(text->vg, 16.0f);
  nvgFillColor(text->vg, nvgRGBA(255, 255, 255, 255));
  nvgTextAlign(text->vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
  nvgText(text->vg, x + w / 2, y + h / 2, label, NULL);

  if (mousePressed && isPointInRect((float)mouseX, (float)mouseY, x, y, w, h)) {
    action();
  }
}
