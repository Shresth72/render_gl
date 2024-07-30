#pragma once
#include <cglm/cglm.h>

float *load_model_from_file(const char *filename, mat4 preTransform,
                            size_t *out_size);
void free_model_memory(float *modelData);
