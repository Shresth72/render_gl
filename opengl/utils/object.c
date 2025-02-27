#include "utils.h"

void get_vertices16(float *vertices, unsigned int *indices, float width,
                    float height, float texWidth, float texHeight) {
  float halfWidth = width / 2.0f;
  float halfHeight = height / 2.0f;

  float tempVertices[] = {
      -halfWidth, -halfHeight, 0.0f,     0.0f,      // Bottom-left
      halfWidth,  -halfHeight, texWidth, 0.0f,      // Bottom-right
      halfWidth,  halfHeight,  texWidth, texHeight, // Top-right
      -halfWidth, halfHeight,  0.0f,     texHeight  // Top-left
  };
  memcpy(vertices, tempVertices, sizeof(tempVertices));

  unsigned int tempIndices[] = {
      0, 1, 2, //
      2, 3, 0  //
  };
  memcpy(indices, tempIndices, sizeof(tempIndices));
}
