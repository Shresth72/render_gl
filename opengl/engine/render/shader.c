#include "shader.h"

Shader *shader_create(const char *vertexPath, const char *fragmentPath,
                      QuadMesh *quad, Texture *texture) {
  Shader *shader = (Shader *)malloc(sizeof(Shader));
  shader->shaderId = 0;
  shader->uniformMap = uniform_create_hashmap();

  // Init Texture and Shader
  if (texture != NULL) {
    shader->texture = texture;
  }

  shader->shaderId = util_load_shader(vertexPath, fragmentPath);
  if (shader->shaderId == 0) {
    free(shader);
    return NULL;
  }

  shader->vertexfilepath = vertexPath;
  shader->fragfilepath = fragmentPath;

  // Bind Shader
  shader_bind(shader);
  // shader_set_uniform4f(shader, "u_Color", 0.6f, 0.2f, 0.5, 1.0f);

  // Bind Texture
  if (shader->texture != NULL) {
    texture_bind(shader->texture, 0);
    shader_set_uniform1i(shader, "u_Texture", 0);
  }

  // Set the QuadMesh
  shader->quad = quad;

  shader->r = 0.0f;
  shader->prevR = 1.0f;
  shader->increment = 0.05f;

  return shader;
}

void shader_destroy(Shader *shader) {
  quadmesh_destroy(shader->quad);
  GLCall(glDeleteProgram(shader->shaderId));
  uniform_hashmap_destroy(shader->uniformMap);
  texture_destroy(shader->texture);
  free(shader);
}

void shader_render(Shader *shader) {
  shader_bind(shader);
  // shader_set_uniform4f(shader, "u_Color", shader->r, 0.2f, 0.5, 1.0f);

  texture_bind(shader->texture, 0);
  shader_set_uniform1i(shader, "u_Texture", 0);

  // Render the quad
  quadmesh_render(shader->quad);

  // if (shader->r > 1.0f) {
  //   shader->increment = -0.05f;
  // } else if (shader->r < 0.0f) {
  //   shader->increment = 0.05f;
  // }
  //
  // shader->r += shader->increment;
}

void shader_bind(Shader *shader) { GLCall(glUseProgram(shader->shaderId)); };

void shader_unbind(Shader *shader) { GLCall(glUseProgram(0)); }

// Uniforms
void shader_set_uniform1i(Shader *shader, const char *name, int value) {
  GLCall(glUniform1i(shader_get_uniform_location(shader, name), value));
}

void shader_set_uniform1f(Shader *shader, const char *name, float value) {
  GLCall(glUniform1i(shader_get_uniform_location(shader, name), value));
}

void shader_set_uniform4f(Shader *shader, const char *name, float v0, float v1,
                          float v2, float v3) {
  GLCall(
      glUniform4f(shader_get_uniform_location(shader, name), v0, v1, v2, v3));
}

int shader_get_uniform_location(Shader *shader, const char *name) {
  int location = uniform_hashmap_get(shader->uniformMap, name);

  if (location == -1) {
    location = glGetUniformLocation(shader->shaderId, name);

    if (location != -1) {
      uniform_hashmap_add(shader->uniformMap, name, location);
    } else {
      printf("Warning: uniform %s doesn't exist\n", name);
    }
  }
  return location;
}

void shader_set_translation(Shader *shader, float x, float y) {
  shader_bind(shader);

  // Create a translation matrix using cglm or any matrix library
  mat4 translation = GLM_MAT4_IDENTITY_INIT; // Initialize to identity matrix
  glm_translate(translation, (vec4){x, y, 0.0f});

  GLCall(
      glUniformMatrix4fv(shader_get_uniform_location(shader, "u_Translation"),
                         1, GL_FALSE, (const GLfloat *)translation));
}

// Load Shader Functions
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
