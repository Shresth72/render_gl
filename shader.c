#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdio.h>
#include <stdlib.h>

#define WIN_WIDTH 1280
#define WIN_HEIGHT 720

GLuint VAO, VBO, shaderProgram;
FT_Library ft;
FT_Face face;
float fontSize = 48.0f;

// Function to compile and link shaders
GLuint createShaderProgram() {
  const char *vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec4 vertex; // <vec4 pos, tex>
        out vec2 TexCoords;
        void main()
        {
            gl_Position = vec4(vertex.xy, 0.0, 1.0);
            TexCoords = vertex.zw;
        }
    )";

  const char *fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoords;
        out vec4 color;
        uniform sampler2D text;
        void main()
        {
            color = texture(text, TexCoords);
        }
    )";

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return program;
}

// Function to initialize FreeType and load a font
int initFreeType() {
  if (FT_Init_FreeType(&ft)) {
    fprintf(stderr, "Could not init FreeType Library\n");
    return -1;
  }

  if (FT_New_Face(ft, "/home/shrestha/.fonts/Inter-Bold.ttf", 0, &face)) {
    fprintf(stderr, "Failed to load font\n");
    return -1;
  }

  FT_Set_Pixel_Sizes(face, 0, fontSize);
  return 0;
}

// Function to initialize OpenGL resources for text rendering
void initOpenGLTextRendering() {
  // Compile and link shaders
  shaderProgram = createShaderProgram();

  // Set up vertex data and buffers
  float vertices[6 * 4] = {
      // Positions   // Texture Coords
      0.0f, 1.0f, 0.0f, 1.0f, // Top-left
      0.0f, 0.0f, 0.0f, 0.0f, // Bottom-left
      1.0f, 0.0f, 1.0f, 0.0f, // Bottom-right

      0.0f, 1.0f, 0.0f, 1.0f, // Top-left
      1.0f, 0.0f, 1.0f, 0.0f, // Bottom-right
      1.0f, 1.0f, 1.0f, 1.0f  // Top-right
  };

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

// Function to render text
void renderText(const char *text, float x, float y) {
  glUseProgram(shaderProgram);
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(VAO);

  for (const char *p = text; *p; p++) {
    if (FT_Load_Char(face, *p, FT_LOAD_RENDER)) {
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

    float xPos = x + face->glyph->bitmap_left;
    float yPos = -y - face->glyph->bitmap_top;
    float w = face->glyph->bitmap.width;
    float h = face->glyph->bitmap.rows;

    float vertices[6 * 4] = {
        xPos, -yPos + h, 0.0f,     1.0f,      xPos,     -yPos,
        0.0f, 0.0f,      xPos + w, -yPos,     1.0f,     0.0f,

        xPos, -yPos + h, 0.0f,     1.0f,      xPos + w, -yPos,
        1.0f, 0.0f,      xPos + w, -yPos + h, 1.0f,     1.0f};

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    x += (face->glyph->advance.x >> 6);
    y += (face->glyph->advance.y >> 6);

    glDeleteTextures(1, &texture);
  }

  glBindVertexArray(0);
  glUseProgram(0);
}

int main() {
  // Initialize the GLFW library
  if (!glfwInit()) {
    return -1;
  }

  // Create a windowed mode window and its OpenGL context
  GLFWwindow *window =
      glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Todo", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  GLenum glewStatus = glewInit();
  if (glewStatus != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW: %s\n",
            glewGetErrorString(glewStatus));
    return -1;
  }

  // Initialize FreeType and OpenGL resources
  if (initFreeType() != 0) {
    return -1;
  }
  initOpenGLTextRendering();

  // Main loop
  int frame_count = 0;
  while (!glfwWindowShouldClose(window)) {
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Set the clear color (background color)
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Begin session
    frame_count++;

    // Print frame count on the window
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Frame count: %d", frame_count);
    renderText(buffer, 10.0f, 100.0f);

    // End session

    // Poll for and process events
    glfwPollEvents();

    // Swap front and back buffers
    glfwSwapBuffers(window);
  }

  // Clean up FreeType and OpenGL resources
  FT_Done_Face(face);
  FT_Done_FreeType(ft);
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteProgram(shaderProgram);

  // Destroy the window and its context
  glfwDestroyWindow(window);

  // Terminate the GLFW library
  glfwTerminate();

  return 0;
}
