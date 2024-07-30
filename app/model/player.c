#include "player.h"

Player *player_create(PlayerCreateInfo *createInfo) {
  Player *player = (Player *)malloc(sizeof(Player));
  if (!player) {
    return NULL;
  }

  glm_vec3_copy(createInfo->position, player->position);
  glm_vec3_copy(createInfo->eulers, player->eulers);

  glm_vec3_copy((vec3){0.0f, 0.0f, 1.0f}, player->up);
  glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, player->forwards);
  glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, player->right);

  glm_mat4_identity(player->viewTransform);

  return player;
}

void player_update(Player *player) {
  // Forwards vector
  player->forwards[0] =
      sin(glm_rad(player->eulers[1])) * cos(glm_rad(player->eulers[2]));
  player->forwards[1] =
      sin(glm_rad(player->eulers[1])) * sin(glm_rad(player->eulers[2]));
  player->forwards[2] = cos(glm_rad(player->eulers[1]));

  vec3 globalUp = {0.0f, 0.0f, 1.0f};

  glm_cross(player->forwards, globalUp, player->right);
  glm_normalize(player->right);
  glm_cross(player->right, player->forwards, player->up);
  glm_normalize(player->up);

  vec3 target;
  glm_vec3_add(player->position, player->forwards, target);
  glm_lookat(player->position, target, player->up, player->viewTransform);
}

void player_destroy(Player *player) { free(player); }
