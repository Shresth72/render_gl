#include "texture.h"
#include "../../../nanovg/src/stb_image.h"

Texture *texture_create(const char *path) {
  Texture *texture = (Texture *)malloc(sizeof(Texture));
  texture->renderId = 0;
  texture->width = 0;
  texture->height = 0;
  texture->BPP = 0;
  texture->localBuffer = NULL;
  texture->filepath = path;

  stbi_set_flip_vertically_on_load(1);
  texture->localBuffer =
      stbi_load(path, &texture->width, &texture->height, &texture->BPP, 4);

  glGenTextures(1, &texture->renderId);
  glBindBuffer(GL_TEXTURE_2D, texture->renderId);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->width, texture->height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, texture->localBuffer);
  glBindTexture(GL_TEXTURE_2D, 0);

  if (texture->localBuffer)
    stbi_image_free(texture->localBuffer);

  return texture;
}

void texture_destroy(Texture *texture) {
  glDeleteTextures(1, &texture->renderId);
}

void texture_bind(Texture *texture, unsigned int slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindBuffer(GL_TEXTURE_2D, texture->renderId);
}

void texture_ubind(Texture *texture, unsigned int slot) {
  glBindTexture(GL_TEXTURE_2D, 0);
}
