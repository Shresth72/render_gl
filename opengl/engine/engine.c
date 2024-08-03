#include "engine.h"

Engine *engine_create(int width, int height, const char *fontPath) {
  Engine *engine = (Engine *)malloc(sizeof(Engine));

  // Initialize Shader Engine
  engine->shaderEngine =
      shader_engine_create("shaders/vertex.glsl", "shaders/fragment.glsl");
  if (!engine->shaderEngine) {
    printf("Could not create shader engine.\n");
    free(engine);
    return NULL;
  }

  // Initialize Text Engine
  engine->textEngine = text_engine_create(fontPath);
  if (!engine->textEngine) {
    shader_engine_destroy(engine->shaderEngine);
    free(engine);
    return NULL;
  }

  return engine;
}

void engine_destroy(Engine *engine) {
  shader_engine_destroy(engine->shaderEngine);
  text_engine_destroy(engine->textEngine);
  free(engine);
}

void engine_render(Engine *engine, int width, int height, int mousePressed,
                   double mouseX, double mouseY) {
  shader_engine_render(engine->shaderEngine);

  // text_engine_render(engine->textEngine, width, height, mousePressed, mouseX,
  //                    mouseY);
}
