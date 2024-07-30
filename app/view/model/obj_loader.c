#include "obj_loader.h"
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#define TINYOBJ_MALLOC my_malloc
#define TINYOBJ_REALLOC my_realloc
#define TINYOBJ_CALLOC my_calloc
#define TINYOBJ_FREE my_free
#include "../../../nanovg/src/tinyobj_loader_c.h"

float *load_model_from_file(const char *filename, mat4 preTransform,
                            size_t *out_size) {
  float *vertices = NULL;
  size_t vertexCount = 0;

  tinyobj_attrib_t attrib;
  tinyobj_shape_t *shapes = NULL;
  size_t shapeCount = 0;
  tinyobj_material_t *materials = NULL;
  size_t materialCount = 0;

  // Load the OBJ file
  tinyobj_parse_obj(&attrib, &shapes, &shapeCount, &materials, &materialCount,
                    filename, NULL, NULL, 0);

  if (attrib.vertices == NULL) {
    fprintf(stderr, "Error loading OBJ: no vertices found\n");
    return NULL;
  }

  // Allocate memory for vertices
  vertexCount =
      shapes[0].mesh.num_face_vertices * 3 * 8; // 8 attributes per vertex
  vertices = (float *)malloc(vertexCount * sizeof(float));
  if (!vertices) {
    fprintf(stderr, "Failed to allocate memory for vertices\n");
    return NULL;
  }

  size_t index = 0;
  for (size_t s = 0; s < shapeCount; ++s) {
    tinyobj_shape_t *shape = &shapes[s];
    for (size_t f = 0; f < shape->mesh.num_face_vertices; ++f) {
      for (size_t v = 0; v < shape->mesh.num_face_vertices; ++v) {
        int idx = shape->mesh.indices[f].vertex_index;

        // Transform position
        vec4 pos = {attrib.vertices[3 * idx], attrib.vertices[3 * idx + 1],
                    attrib.vertices[3 * idx + 2], 1.0f};
        glm_mat4_mulv(preTransform, pos, pos);

        // Add position and other attributes to vertices
        vertices[index++] = pos[0]; // x
        vertices[index++] = pos[1]; // y
        vertices[index++] = pos[2]; // z

        // Add texCoords (if they exist)
        if (attrib.texcoords) {
          vertices[index++] = attrib.texcoords[2 * idx];
          vertices[index++] = attrib.texcoords[2 * idx + 1];
        } else {
          vertices[index++] = 0.0f; // default texCoord
          vertices[index++] = 0.0f;
        }

        // Add normals (if they exist)
        if (attrib.normals) {
          vertices[index++] = attrib.normals[3 * idx];
          vertices[index++] = attrib.normals[3 * idx + 1];
          vertices[index++] = attrib.normals[3 * idx + 2];
        } else {
          vertices[index++] = 0.0f; // default normal
          vertices[index++] = 0.0f;
          vertices[index++] = 0.0f;
        }
      }
    }
  }

  *out_size = vertexCount;
  return vertices;
}

void free_model_memory(float *modelData) { free(modelData); }
