#include "engine.h"

Engine *engine_create(int width, int height) {
  Engine *engine = (Engine *)malloc(sizeof(Engine));

  engine->shader =
      util_load_shader("shaders/vertex.glsl", "shaders/fragment.glsl");
  if (engine->shader == 0) {
    free(engine);
    return NULL;
  }

  engine_create_models(engine);

  return engine;
}

void engine_destroy(Engine *engine) {
  quadmesh_destroy(engine->ourQuad);
  GLCall(glDeleteProgram(engine->shader));

  free(engine);
}

void engine_create_models(Engine *engine) {
  engine->ourQuad = quadmesh_create(1.0f, 1.0f);
}

void engine_render(Engine *engine, int width, int height) {
  GLCall(glViewport(0, 0, width, height));
  GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

  engine_shader_render(engine);
}

void engine_shader_render(Engine *engine) {
  GLCall(glUseProgram(engine->shader));
  quadmesh_render(engine->ourQuad);
}
