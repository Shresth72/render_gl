#define NANOVG_GL3_IMPLEMENTATION
#include "engine.h"

NVGcontext *initializeNanoVG(const char *fontPath) {
  NVGcontext *vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
  if (vg == NULL) {
    return NULL;
  }

  int font = nvgCreateFont(vg, "sans", fontPath);
  if (font == -1) {
    printf("Could not add font.\n");
    nvgDeleteGL3(vg);
    return NULL;
  }

  return vg;
}

Engine *engine_create(const char *fontPath) {
  Engine *engine = (Engine *)malloc(sizeof(Engine));

  engine->shader =
      util_load_shader("shaders/vertex.glsl", "shaders/fragment.glsl");
  if (engine->shader == 0) {
    free(engine);
    return NULL;
  }

  engine_create_models(engine);

  engine->vg = initializeNanoVG(fontPath);
  if (engine->vg == NULL) {
    printf("Could not init nanovg.\n");
    free(engine);
    return NULL;
  }

  return engine;
}

void engine_destroy(Engine *engine) {
  quadmesh_destroy(engine->ourQuad);
  free(engine);
}

void engine_create_models(Engine *engine) {
  engine->ourQuad = quadmesh_create(1.0f, 1.0f);
}

void engine_render(Engine *engine, int width, int height, int mousePressed,
                   double mouseX, double mouseY) {
  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT);

  engine_shader_render(engine);
  engine_text_render(engine, width, height, mousePressed, mouseX, mouseY);
}

void engine_shader_render(Engine *engine) {
  glUseProgram(engine->shader);
  glBindVertexArray(engine->ourQuad->VAO);
  glDrawArrays(GL_TRIANGLES, 0, engine->ourQuad->vertexCount);
  glBindVertexArray(0);
}

int isPointInRect(float px, float py, float x, float y, float w, float h) {
  return px >= x && px <= x + w && py >= y && py <= y + h;
}

void engine_text_render(Engine *engine, int width, int height, int mousePressed,
                        double mouseX, double mouseY) {
  nvgBeginFrame(engine->vg, width, height, 1.0f);

  // Draw Text
  nvgFontFace(engine->vg, "sans");
  nvgFontSize(engine->vg, 48.0f);
  nvgFillColor(engine->vg, nvgRGBA(255, 255, 255, 255));
  nvgTextAlign(engine->vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
  nvgText(engine->vg, 10.0f, 8.0f, "Hello, NanoVG!", NULL);

  // Draw Button
  float btnW = 120, btnH = 30;
  float btnX = (width - btnW) / 2, btnY = (height - btnH) / 2;
  nvgBeginPath(engine->vg);
  nvgRoundedRect(engine->vg, btnX, btnY, btnW, btnH, 10.f);
  nvgFillColor(engine->vg, nvgRGB(0, 128, 255));
  nvgFill(engine->vg);
  nvgStrokeColor(engine->vg, nvgRGB(0, 0, 0));
  nvgStroke(engine->vg);

  nvgFontSize(engine->vg, 20.0f);
  nvgFillColor(engine->vg, nvgRGBA(255, 255, 255, 255));
  nvgTextAlign(engine->vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
  nvgText(engine->vg, btnX + btnW / 2, btnY + btnH / 2, "Close", NULL);

  if (mousePressed &&
      isPointInRect((float)mouseX, (float)mouseY, btnX, btnY, btnW, btnH)) {
    glfwSetWindowShouldClose(glfwGetCurrentContext(), 1);
  }

  nvgEndFrame(engine->vg);
}
