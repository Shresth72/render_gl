#include "engine.h"

Engine *engine_create(int width, int height, const char *fontPath) {
  Engine *engine = (Engine *)malloc(sizeof(Engine));

  // Initialize Shader Engine
  engine->shader =
      shader_create("shaders/ink/vertex.glsl", "shaders/ink/fragment.glsl");
  if (!engine->shader) {
    printf("Could not create shader engine.\n");
    free(engine);
    return NULL;
  }

  // Initialize Text Engine
  engine->text = text_create(fontPath);
  if (!engine->text) {
    shader_destroy(engine->shader);
    free(engine);
    return NULL;
  }

  return engine;
}

void engine_destroy(Engine *engine) {
  shader_destroy(engine->shader);
  text_destroy(engine->text);
  free(engine);
}

void engine_render(Engine *engine, int width, int height, int mousePressed,
                   double mouseX, double mouseY) {
  shader_render(engine->shader);

  text_render(engine->text, width, height, mousePressed, mouseX, mouseY);
}
