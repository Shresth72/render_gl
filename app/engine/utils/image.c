#include "image.h"
#include "../../nanovg/src/stb_image.h"

Image load_from_file(const char *filename) {
  Image result;

  result.data = stbi_load(filename, &(result.width), &(result.height), NULL,
                          STBI_rgb_alpha);

  if (!result.data) {
    result.width = 0;
    result.height = 0;
  }

  return result;
}

void free_image_memory(Image oldImage) { stbi_image_free(oldImage.data); }
