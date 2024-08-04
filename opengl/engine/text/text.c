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

int isPointInRect(float px, float py, float x, float y, float w, float h) {
  return px >= x && px <= x + w && py >= y && py <= y + h;
}

void text_render(Text *text, int width, int height, int mousePressed,
                 double mouseX, double mouseY) {
  nvgBeginFrame(text->vg, width, height, 1.0f);

  // Draw Text
  nvgFontFace(text->vg, "sans");
  nvgFontSize(text->vg, 48.0f);
  nvgFillColor(text->vg, nvgRGBA(255, 255, 255, 255));
  nvgTextAlign(text->vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
  nvgText(text->vg, 10.0f, 8.0f, "Hello, NanoVG!", NULL);

  // Draw Button
  float btnW = 120, btnH = 30;
  float btnX = (width - btnW) / 2, btnY = (height - btnH) / 2;
  nvgBeginPath(text->vg);
  nvgRoundedRect(text->vg, btnX, btnY, btnW, btnH, 10.f);
  nvgFillColor(text->vg, nvgRGB(0, 128, 255));
  nvgFill(text->vg);
  nvgStrokeColor(text->vg, nvgRGB(0, 0, 0));
  nvgStroke(text->vg);

  nvgFontSize(text->vg, 20.0f);
  nvgFillColor(text->vg, nvgRGBA(255, 255, 255, 255));
  nvgTextAlign(text->vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
  nvgText(text->vg, btnX + btnW / 2, btnY + btnH / 2, "Close", NULL);

  if (mousePressed &&
      isPointInRect((float)mouseX, (float)mouseY, btnX, btnY, btnW, btnH)) {
    GLCall(glfwSetWindowShouldClose(glfwGetCurrentContext(), 1));
  }

  nvgEndFrame(text->vg);
}
