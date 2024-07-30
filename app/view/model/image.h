#pragma once
#include "stdio.h"
#include "stdlib.h"

typedef struct {
  int width;
  int height;
  unsigned char *data;
} Image;

Image load_from_file(const char *filename);
void free_image_memory(Image oldImage);
