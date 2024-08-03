#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  unsigned int renderId;
  const char *filepath;
  unsigned char *localBuffer;
  int width, height, BPP;
} Texture;

Texture *texture_create(const char *path);
void texture_destroy(Texture *texture);
void texture_bind(Texture *texture, unsigned int slot);
void texture_ubind(Texture *texture, unsigned int slot);
