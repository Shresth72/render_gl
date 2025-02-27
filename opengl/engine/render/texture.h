#pragma once
#include "../../../nanovg/src/stb_image.h"
#include "../../utils/utils.h"
#include <stdlib.h>

typedef struct {
  unsigned int rendererId;

  const char *filepath;
  unsigned char *localBuffer;

  int width, height, bpp;
} Texture;

Texture *texture_create(const char *path);
void texture_destroy(Texture *texture);
void texture_bind(Texture *texture, unsigned int slot);
void texture_unbind(Texture *texture);
