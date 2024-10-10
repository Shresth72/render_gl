#include "tank.h"

TankObject *tank_object_create() {
  TankObject *tankObj = (TankObject *)malloc(sizeof(TankObject));
  if (!tankObj)
    return NULL;

  float vertices[] = {
      // x      y        texCoordX texCoordY
      -0.125f, -0.25f, 0.0f, 0.0f, // Bottom-left
      0.125f,  -0.25f, 1.0f, 0.0f, // Bottom-right
      0.125f,  0.25f,  1.0f, 1.0f, // Top-right
      -0.125f, 0.25f,  0.0f, 1.0f  // Top-left
  };

  unsigned int indices[] = {
      0, 1, 2, // First triangle
      2, 3, 0  // Second triangle
  };

  size_t vertexCount = sizeof(vertices) / sizeof(vertices[0]);
  size_t indexCount = sizeof(indices) / sizeof(indices[0]);

  tankObj->quadMesh =
      quadmesh_create(vertices, indices, vertexCount, indexCount);
  if (!tankObj->quadMesh) {
    free(tankObj);
    return NULL;
  }

  tankObj->texture = texture_create("textures/kavneet.jpg");
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

  initialize_tank_defaults(tankObj);

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
  shader_set_translation(tankObj->shader, tankObj->currX, tankObj->currY);
  shader_render(tankObj->shader);
}

void tank_object_move(TankObject *tankObj, float offsetX, float offsetY,
                      int spacePressed) {
  tankObj->currX += offsetX;

  if (offsetY != 0.0f) {
    tankObj->currY += offsetY;
    tankObj->groundLevel = tankObj->currY;
  }

  // Jumping
  if (spacePressed && !tankObj->isJumping) {
    tankObj->isJumping = 1;
    tankObj->jumpVelocity = 0.03f;
  }

  if (tankObj->isJumping) {
    tankObj->currY += tankObj->jumpVelocity;
    tankObj->jumpVelocity += tankObj->gravity;

    if (tankObj->currY <= tankObj->groundLevel) {
      tankObj->currY = tankObj->groundLevel;
      tankObj->isJumping = 0;
      tankObj->jumpVelocity = 0.0f;
    }
  } else {
    // If not jumping, allow other vertical movements if needed
    if (offsetY != 0.0f) {
      tankObj->currY += offsetY;
    }
  }
}

void initialize_tank_defaults(TankObject *tankObj) {
  tankObj->currX = 0.0f;
  tankObj->currY = 0.0f;

  tankObj->jumpVelocity = 0.0f;
  tankObj->isJumping = 0;
  tankObj->gravity = -0.001f;
  tankObj->groundLevel = 0.0f;

  tankObj->boundingBox.width =
      calculate_quad_width(tankObj->quadMesh->vertices);
  tankObj->boundingBox.height =
      calculate_quad_height(tankObj->quadMesh->vertices);
  tankObj->boundingBox.x = tankObj->currX - (tankObj->boundingBox.width / 2);
  tankObj->boundingBox.y = tankObj->currY - (tankObj->boundingBox.height / 2);
}

float calculate_quad_width(float *vertices) {
  return fabs(vertices[2] - vertices[0]); // Bottom-right x - Bottom-left x
}

float calculate_quad_height(float *vertices) {
  return fabs(vertices[5] - vertices[1]); // Top-left y - Bottom-left y
}
