#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include "linmath.h"
#include <stdio.h>
#include <stdlib.h>

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

#define WIN_MARGIN 20.0f

static int winw = 1280, winh = 720;
static int frame_count = 0;

struct Character {
  GLuint TextureID;
  int Size[2];
  int Bearing[2];
  GLuint Advance;
};
struct Character Characters[128];

GLuint VAO, VBO, shaderProgram, projectionLoc, textColorLoc;

// Compiler and link the shaderProgram
GLuint setupShaders() {
  GLuint vertexShader, fragmentShader;

  // Vertex Shader
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    printf("error: shader: vertex compilation failed\n%s\n", infoLog);
  }

  // Fragment Shader
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    printf("error: shader: fragment compilation failed\n%s\n", infoLog);
  }

  // Shader Program
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    printf("error: shader: program linking failed\n%s\n", infoLog);
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}

// Inittialize FreeType and Load Font
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

  // Load character glyphs
  for (unsigned char c = 0; c < 128; c++) {
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      printf("error: freetype: failed to load glyph\n");
      continue;
    }

    // Generate texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width,
                 face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                 face->glyph->bitmap.buffer);

    // Set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Store character for later use
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

// Setup vertex data and buffer and confihure vertex attributes
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

// Render text
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
        {xpos, ypos + h, 0.0, 0.0}, {xpos, ypos, 0.0, 1.0},
        {xpos + w, ypos, 1.0, 1.0}, {xpos, ypos + h, 0.0, 0.0},
        {xpos + w, ypos, 1.0, 1.0}, {xpos + w, ypos + h, 1.0, 0.0},
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

int main() {
  if (!glfwInit()) {
    printf("error: glfw: failed to initialize\n");
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(winw, winh, "Todo", NULL, NULL);
  if (!window) {
    glfwTerminate();
    printf("error: glfw: failed to create window\n");
    return -1;
  }

  glfwMakeContextCurrent(window);
  if (glewInit() != GLEW_OK) {
    printf("error: glew: failed to initialize\n");
    return -1;
  }

  // Compile and setup the shader
  shaderProgram = setupShaders();
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

  frame_count = 0;

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    // Nvim Colors: 0.117647f, 0.117647f, 0.180392f, 1.0f
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    frame_count++;

    // print something on the window session
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Todo Frame Count: %d", frame_count);
    float color[3] = {0.5f, 0.8f, 0.2f};

    renderText(buffer, 10.0f, winh - 50.0f, 1.0f, color);

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
