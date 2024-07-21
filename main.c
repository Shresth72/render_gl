#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <freetype2/ft2build.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include FT_FREETYPE_H
#include "linmath.h"

GLuint VAO, VBO, shaderProgram, projectionLoc, textColorLoc;
static int winw = 1280, winh = 720;

struct Character {
  GLuint TextureID;
  int Size[2];
  int Bearing[2];
  GLuint Advance;
};
struct Character Characters[128];

// Shader sources
const char *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec4 vertex;\n"
    "out vec2 TexCoords;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
    "    TexCoords = vertex.zw;\n"
    "}\0";

const char *fragmentShaderSource =
    "#version 330 core\n"
    "in vec2 TexCoords;\n"
    "out vec4 color;\n"
    "uniform sampler2D text;\n"
    "uniform vec3 textColor;\n"
    "void main()\n"
    "{\n"
    "    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
    "    color = vec4(textColor, 1.0) * sampled;\n"
    "}\n\0";

static unsigned int compileShader(unsigned int type, const char *source) {
  unsigned int id = glCreateShader(type);
  glShaderSource(id, 1, &source, NULL);
  glCompileShader(id);

  int result = 0;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);

  if (result == GL_FALSE) {
    int length = 0;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    char *message = (char *)alloca(length);
    glGetShaderInfoLog(id, length, &length, message);

    printf("Failed to compile %s shader\n",
           (type == GL_VERTEX_SHADER) ? "vertex" : "fragment");
    printf("%s\n", message);

    glDeleteShader(id);
    return 0; // Return 0 to indicate failure
  }

  return id;
}

static unsigned int createShader(const char *vertexShaderSource,
                                 const char *fragmentShaderSource) {
  unsigned int program = glCreateProgram();
  unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
  unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glValidateProgram(program);

  glDeleteShader(vs);
  glDeleteShader(fs);

  return program;
}

void setupBuffers() {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void initFreeType() {
  FT_Library ft;
  if (FT_Init_FreeType(&ft)) {
    printf("error: freetype: could not init FreeType Library\n");
    return;
  }

  FT_Face face;
  if (FT_New_Face(ft, "/home/shrestha/.fonts/Inter-Bold.ttf", 0, &face)) {
    printf("error: freetype: failed to load font\n");
    return;
  }

  FT_Set_Pixel_Sizes(face, 0, 48);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  for (unsigned char c = 0; c < 128; c++) {
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      printf("error: freetype: failed to load glyph\n");
      continue;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width,
                 face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                 face->glyph->bitmap.buffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    struct Character character = {
        texture,
        {face->glyph->bitmap.width, face->glyph->bitmap.rows},
        {face->glyph->bitmap_left, face->glyph->bitmap_top},
        (GLuint)face->glyph->advance.x};
    Characters[c] = character;
  }

  FT_Done_Face(face);
  FT_Done_FreeType(ft);
}

void renderText(const char *text, float x, float y, float scale,
                float color[3]) {
  glUseProgram(shaderProgram);
  glUniform3f(textColorLoc, color[0], color[1], color[2]);
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(VAO);

  // Iterate through all characters
  for (const char *c = text; *c; c++) {
    struct Character ch = Characters[*c];

    float xpos = x + ch.Bearing[0] * scale;
    float ypos = y - (ch.Size[1] - ch.Bearing[1]) * scale;

    float w = ch.Size[0] * scale;
    float h = ch.Size[1] * scale;

    // Update VBO for each character
    float vertices[6][4] = {
        {xpos, ypos + h, 0.0f, 0.0f}, {xpos, ypos, 0.0f, 1.0f},
        {xpos + w, ypos, 1.0f, 1.0f}, {xpos, ypos + h, 0.0f, 0.0f},
        {xpos + w, ypos, 1.0f, 1.0f}, {xpos + w, ypos + h, 1.0f, 0.0f},
    };

    // Render glyph texture over quad
    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
    // Update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Render quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // Advance cursors for next glyph
    x += (ch.Advance >> 6) * scale;
  }

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

int main(void) {
  GLFWwindow *window;

  // Initialize the library
  if (!glfwInit())
    return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create a windowed mode window and its OpenGL context
  window = glfwCreateWindow(winw, winh, "Shader", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);
  if (glewInit() != GLEW_OK) {
    printf("Error initializing GLEW\n");
    return -1;
  }

  printf("%s\n", glGetString(GL_VERSION));

  // Setup Buffers
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);

  shaderProgram = createShader(vertexShaderSource, fragmentShaderSource);
  glUseProgram(shaderProgram);

  // Set orthographic projection matrix
  projectionLoc = glGetUniformLocation(shaderProgram, "projection");
  mat4x4 projection;
  mat4x4_ortho(projection, 0.0f, winw, 0.0f, winh, -1.0f, 1.0f);
  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (const GLfloat *)projection);

  textColorLoc = glGetUniformLocation(shaderProgram, "textColor");

  // Initialize FreeType
  initFreeType();

  // Setup buffers
  setupBuffers();

  // Loop until the user closes the window
  while (!glfwWindowShouldClose(window)) {
    // Render here
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Render This Text");
    float color[3] = {0.5f, 0.8f, 0.2f};

    renderText(buffer, 10.0f, winh - 50.0f, 1.0f, color);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Cleanup
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  glfwTerminate();
  return 0;
}
