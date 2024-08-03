#include "material.h"

void check_gl_error(const char *label) {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    printf("OpenGL Error @ %s: %d\n", label, error);
  }
}

Material *material_create(MaterialCreateInfo *createInfo) {
  Material *material = (Material *)malloc(sizeof(Material));
  if (!material) {
    return NULL;
  }

  int texWidth, texHeight;
  Image img = load_from_file(createInfo->filename);
  texWidth = img.width;
  texHeight = img.height;
  unsigned char *data = img.data;

  if (!data) {
    printf("Failed to load texture from file: %s\n", createInfo->filename);
    free(material);
    return NULL;
  }
  printf("%d - %d\n", texHeight, texWidth);

  check_gl_error("done");
  glGenTextures(1, &material->texture);
  check_gl_error("Gen texture");
  glBindTexture(GL_TEXTURE_2D, material->texture);
  check_gl_error("bind texture");
  glTextureSubImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0,
                      GL_RGBA, GL_UNSIGNED_BYTE, data);
  check_gl_error("tex sub imag3");

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
