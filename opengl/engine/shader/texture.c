#include "texture.h"

Texture *texture_create(const char *path) {
  Texture *texture = (Texture *)malloc(sizeof(Texture));
  texture->filepath = path;
  texture->localBuffer = NULL;
  texture->rendererId = texture->width = texture->height = texture->bpp = 0;

  stbi_set_flip_vertically_on_load(1);
  texture->localBuffer =
      stbi_load(path, &texture->width, &texture->height, &texture->bpp, 4);

  GLCall(glGenTextures(1, &texture->rendererId));
  GLCall(glBindTexture(GL_TEXTURE_2D, texture->rendererId));

  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

  GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->width,
                      texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                      texture->localBuffer));
  GLCall(glBindTexture(GL_TEXTURE_2D, 0));

  if (texture->localBuffer)
    stbi_image_free(texture->localBuffer);

  return texture;
}

void texture_destroy(Texture *texture) {
  GLCall(glDeleteTextures(1, &texture->rendererId));
  free(texture);
}

void texture_bind(Texture *texture, unsigned int slot) {
  GLCall(glActiveTexture(GL_TEXTURE0 + slot));
  GLCall(glBindTexture(GL_TEXTURE_2D, texture->rendererId));
}

void texture_unbind(Texture *texture) {
  GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}
