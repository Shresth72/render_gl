#pragma once
#include "../utils/errors.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

unsigned int util_load_shader(const char *vertexFilepath,
                              const char *fragmentFilepath);
