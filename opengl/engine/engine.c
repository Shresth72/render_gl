#include "engine.h"

Engine *engine_create(int width, int height, const char *fontPath) {
  Engine *engine = (Engine *)malloc(sizeof(Engine));

  // Initialize Ink Object
  engine->inkObj = ink_object_create();
  if (!engine->inkObj) {
    printf("Could not create shader engine.\n");
    engine_destroy(engine);
    return NULL;
  }

  // Initialize Tank Object
  engine->tankObj = tank_object_create();
  if (!engine->tankObj) {
    printf("Could not create tank engine.\n");
    engine_destroy(engine);
    return NULL;
  }

  // Initialize Text Engine
  engine->text = text_create(fontPath);
  if (!engine->text) {
    engine_destroy(engine);
    return NULL;
  }

  // Init default values
  engine->spacePressed = 0;

  return engine;
}

void engine_destroy(Engine *engine) {
  ink_object_destroy(engine->inkObj);
  tank_object_destroy(engine->tankObj);
  text_destroy(engine->text);
  free(engine);
}

void engine_render(Engine *engine, int width, int height, int mousePressed) {
  ink_object_render(engine->inkObj);

  tank_object_render(engine->tankObj, engine->moveX, engine->moveY,
                     engine->spacePressed);

  text_render(engine->text, width, height, mousePressed, engine->mouseX,
              engine->mouseY);
}
