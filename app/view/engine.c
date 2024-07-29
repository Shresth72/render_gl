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
  free(engine);
}

void engine_create_models(Engine *engine) {
  engine->ourQuad = quadmesh_create(1.0f, 1.0f);
}

void engine_render(Engine *engine) {
  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(engine->shader);
  glBindVertexArray(engine->ourQuad->VAO);
  glDrawArrays(GL_TRIANGLES, 0, engine->ourQuad->vertexCount);
}
