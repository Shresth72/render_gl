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
  glDeleteProgram(engine->shader);

  free(engine);
}

void engine_create_models(Engine *engine) {
  engine->ourQuad = quadmesh_create(1.0f, 1.0f);
}

void engine_render(Engine *engine, int width, int height) {
  glViewport(0, 0, width, height);
  // glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  engine_shader_render(engine);
}

void engine_shader_render(Engine *engine) {
  glUseProgram(engine->shader);
  // glBindBufferRange(GL_UNIFORM_BUFFER, engine->blockIndex, engine->UBO, 0,
  //                   engine->blockSize);
  quadmesh_render(engine->ourQuad);
}
