#include "engine.h"

Engine *engine_create(int width, int height, const char *fontPath) {
  Engine *engine = (Engine *)malloc(sizeof(Engine));

  // Initialize Ink Object
  engine->inkObj = ink_object_create();
  if (!engine->inkObj) {
    printf("Could not create shader engine.\n");
    ink_object_destroy(engine->inkObj);
    free(engine);
    return NULL;
  }

  // Initialize Tank Object
  engine->tankObj = tank_object_create();
  if (!engine->tankObj) {
    printf("Could not create tank engine.\n");
    ink_object_destroy(engine->inkObj);
    free(engine);
    return NULL;
  }

  // Initialize Text Engine
  engine->text = text_create(fontPath);
  if (!engine->text) {
    ink_object_destroy(engine->inkObj);
    free(engine);
    return NULL;
  }

  return engine;
}

void engine_destroy(Engine *engine) {
  ink_object_destroy(engine->inkObj);
  tank_object_destroy(engine->tankObj);
  text_destroy(engine->text);
  free(engine);
}

void engine_render(Engine *engine, int width, int height, int mousePressed,
                   double mouseX, double mouseY, float moveX, float moveY) {
  ink_object_render(engine->inkObj);

  // Update tank position
  tank_object_move(engine->tankObj, moveX, moveY);
  tank_object_render(engine->tankObj);

  text_render(engine->text, width, height, mousePressed, mouseX, mouseY);
}
