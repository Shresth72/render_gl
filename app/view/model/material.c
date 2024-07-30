#include "material.h"

Material *material_create(MaterialCreateInfo *createInfo) {
  Material *material = (Material *)malloc(sizeof(Material));
  if (!material) {
    return NULL;
  }

  int texWidth, texHeight;
  Image img = load_from_file(createInfo->filename);
  texWidth = img.width;
  texHeight = img.height;

  if (img.data == NULL) {
    free(material);
    return NULL;
  }

  glCreateTextures(GL_TEXTURE_2D, 1, &material->texture);
  glTextureStorage2D(material->texture, 1, GL_RGBA8, texWidth, texHeight);
  glTextureSubImage2D(material->texture, 0, 0, 0, texWidth, texHeight, GL_RGBA,
                      GL_UNSIGNED_BYTE, img.data);
  glTextureParameteri(material->texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTextureParameteri(material->texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTextureParameteri(material->texture, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR_MIPMAP_LINEAR);
  glTextureParameteri(material->texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  free_image_memory(img);

  return material;
}

void material_destroy(Material *material) {
  if (material) {
    glDeleteTextures(1, &material->texture);
    free(material);
  }
}

void material_use(Material *material) {
  glBindTextureUnit(0, material->texture);
}
