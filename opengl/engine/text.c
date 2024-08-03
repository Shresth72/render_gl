#define NANOVG_GL3_IMPLEMENTATION
#include "text.h"

TextEngine *text_engine_create(const char *fontPath) {
  TextEngine *textEngine = (TextEngine *)malloc(sizeof(TextEngine));
  textEngine->vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);

  if (textEngine->vg == NULL) {
    free(textEngine);
    return NULL;
  }

  int font = nvgCreateFont(textEngine->vg, "sans", fontPath);
  if (font == -1) {
    printf("Could not add font.\n");
    nvgDeleteGL3(textEngine->vg);
    free(textEngine);
    return NULL;
  }

  return textEngine;
}

void text_engine_destroy(TextEngine *textEngine) {
  nvgDeleteGL3(textEngine->vg);
  free(textEngine);
}

int isPointInRect(float px, float py, float x, float y, float w, float h) {
  return px >= x && px <= x + w && py >= y && py <= y + h;
}

void text_engine_render(TextEngine *textEngine, int width, int height,
                        int mousePressed, double mouseX, double mouseY) {
  nvgBeginFrame(textEngine->vg, width, height, 1.0f);

  // Draw Text
  nvgFontFace(textEngine->vg, "sans");
  nvgFontSize(textEngine->vg, 48.0f);
  nvgFillColor(textEngine->vg, nvgRGBA(255, 255, 255, 255));
  nvgTextAlign(textEngine->vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
  nvgText(textEngine->vg, 10.0f, 8.0f, "Hello, NanoVG!", NULL);

  // Draw Button
  float btnW = 120, btnH = 30;
  float btnX = (width - btnW) / 2, btnY = (height - btnH) / 2;
  nvgBeginPath(textEngine->vg);
  nvgRoundedRect(textEngine->vg, btnX, btnY, btnW, btnH, 10.f);
  nvgFillColor(textEngine->vg, nvgRGB(0, 128, 255));
  nvgFill(textEngine->vg);
  nvgStrokeColor(textEngine->vg, nvgRGB(0, 0, 0));
  nvgStroke(textEngine->vg);

  nvgFontSize(textEngine->vg, 20.0f);
  nvgFillColor(textEngine->vg, nvgRGBA(255, 255, 255, 255));
  nvgTextAlign(textEngine->vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
  nvgText(textEngine->vg, btnX + btnW / 2, btnY + btnH / 2, "Close", NULL);

  if (mousePressed &&
      isPointInRect((float)mouseX, (float)mouseY, btnX, btnY, btnW, btnH)) {
    GLCall(glfwSetWindowShouldClose(glfwGetCurrentContext(), 1));
  }

  nvgEndFrame(textEngine->vg);
}
