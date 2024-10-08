#include "tank.h"
#include <stdio.h>

TankObject *tank_object_create() {
  TankObject *tankObj = (TankObject *)malloc(sizeof(TankObject));
  if (!tankObj)
    return NULL;

  tankObj->currX = 0.0f;
  tankObj->currY = 0.0f;

  float vertices[] = {
      -0.125f, -0.125f, 0.0f, 0.0f, // Bottom-left
      0.125f,  -0.125f, 1.0f, 0.0f, // Bottom-right
      0.125f,  0.125f,  1.0f, 1.0f, // Top-right
      -0.125f, 0.125f,  0.0f, 1.0f  // Top-left
  };

  unsigned int indices[] = {
      0, 1, 2, //
      2, 3, 0, //
  };

  size_t vertexCount = sizeof(vertices) / sizeof(vertices[0]);
  size_t indexCount = sizeof(indices) / sizeof(indices[0]);

  tankObj->quadMesh =
      quadmesh_create(vertices, indices, vertexCount, indexCount);
  if (!tankObj->quadMesh) {
    free(tankObj);
    return NULL;
  }

  tankObj->texture = texture_create("textures/tank.png");
  if (!tankObj->texture) {
    quadmesh_destroy(tankObj->quadMesh);
    free(tankObj);
    return NULL;
  }

  tankObj->shader =
      shader_create("shaders/tank/vertex.glsl", "shaders/tank/fragment.glsl",
                    tankObj->quadMesh, tankObj->texture);
  if (!tankObj->shader) {
    texture_destroy(tankObj->texture);
    quadmesh_destroy(tankObj->quadMesh);
    free(tankObj);
    return NULL;
  }

  return tankObj;
}

void tank_object_destroy(TankObject *tankObj) {
  if (tankObj) {
    shader_destroy(tankObj->shader);
    texture_destroy(tankObj->texture);
    quadmesh_destroy(tankObj->quadMesh);
    free(tankObj);
  }
}

void tank_object_render(TankObject *tankObj) {
  // Apply translation to the tank's position
  shader_set_translation(tankObj->shader, tankObj->currX, tankObj->currY);

  shader_render(tankObj->shader);
}

void tank_object_move(TankObject *tankObj, float offsetX, float offsetY) {
  if (offsetX != 0.0f || offsetY != 0.0f) {
    tankObj->currX += offsetX;
    tankObj->currY += offsetY;
  }
}
