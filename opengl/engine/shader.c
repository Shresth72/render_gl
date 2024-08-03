#include "shader.h"

ShaderEngine *shader_engine_create(const char *vertexPath,
                                   const char *fragmentPath) {
  ShaderEngine *shaderEngine = (ShaderEngine *)malloc(sizeof(ShaderEngine));

  // Load Shader
  shaderEngine->shader = util_load_shader(vertexPath, fragmentPath);
  if (shaderEngine->shader == 0) {
    free(shaderEngine);
    return NULL;
  }

  GLCall(glUseProgram(shaderEngine->shader));
  GLCall(shaderEngine->location =
             glGetUniformLocation(shaderEngine->shader, "u_Color"));
  ASSERT(shaderEngine->location != -1);
  GLCall(glUniform4f(shaderEngine->location, 0.2f, 0.3f, 0.8f, 1.0f));

  // Create QuadMesh
  shaderEngine->ourQuad = quadmesh_create(1.0f, 1.0f);
  if (shaderEngine->ourQuad == NULL) {
    glDeleteProgram(shaderEngine->shader);
    free(shaderEngine);
    return NULL;
  }

  shaderEngine->r = 0.0f;
  shaderEngine->increment = 0.05f;

  return shaderEngine;
}

void shader_engine_destroy(ShaderEngine *shaderEngine) {
  quadmesh_destroy(shaderEngine->ourQuad); // Destroy the QuadMesh
  GLCall(glDeleteProgram(shaderEngine->shader));
  free(shaderEngine);
}

void shader_engine_render(ShaderEngine *shaderEngine) {
  GLCall(glUseProgram(shaderEngine->shader));
  GLCall(glUniform4f(shaderEngine->location, 0.5f, 0.7f, 0.9f, 1.0f));

  // Render the quad
  quadmesh_render(shaderEngine->ourQuad);

  // if (shaderEngine->r > 1.0f) {
  //   shaderEngine->increment = -0.05f;
  // } else if (shaderEngine->r < 0.0f) {
  //   shaderEngine->increment = 0.05f;
  // }
  //
  // shaderEngine->r += shaderEngine->increment;
}

static char *load_shader_source(const char *filePath) {
  FILE *file = fopen(filePath, "r");
  if (!file) {
    fprintf(stderr, "Failed to open shader file: %s\n", filePath);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *source = (char *)malloc(length + 1);
  if (!source) {
    fprintf(stderr, "Failed to allocate memory for shader source\n");
    fclose(file);
    return NULL;
  }

  fread(source, 1, length, file);
  source[length] = '\0';

  fclose(file);
  return source;
}

static unsigned int compile_shader(unsigned int type, const char *source) {
  unsigned int id = glCreateShader(type);
  GLCall(glShaderSource(id, 1, &source, NULL));
  GLCall(glCompileShader(id));

  int result;
  GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
  if (result == GL_FALSE) {
    int length;
    GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
    char *message = (char *)alloca(length);
    GLCall(glGetShaderInfoLog(id, length, &length, message));
    fprintf(stderr, "Failed to compile %s shader\n",
            (type == GL_VERTEX_SHADER) ? "vertex" : "fragment");
    fprintf(stderr, "%s\n", message);
    GLCall(glDeleteShader(id));
    return 0;
  }

  return id;
}

unsigned int util_load_shader(const char *vertexFilepath,
                              const char *fragmentFilepath) {
  char *vertexSource = load_shader_source(vertexFilepath);
  char *fragmentSource = load_shader_source(fragmentFilepath);

  if (!vertexSource || !fragmentSource) {
    free(vertexSource);
    free(fragmentSource);
    return 0;
  }

  unsigned int program = glCreateProgram();
  unsigned int vs = compile_shader(GL_VERTEX_SHADER, vertexSource);
  unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, fragmentSource);

  GLCall(glAttachShader(program, vs));
  GLCall(glAttachShader(program, fs));
  GLCall(glLinkProgram(program));
  GLCall(glValidateProgram(program));

  GLCall(glDeleteShader(vs));
  GLCall(glDeleteShader(fs));

  free(vertexSource);
  free(fragmentSource);

  return program;
}
