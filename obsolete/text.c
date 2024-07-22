#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <freetype2/ft2build.h>
#include <stdio.h>
#include FT_FREETYPE_H

static int winw = 1280, winh = 720;

const char *vertexShaderSource = "#version 120\n"
                                 "attribute vec4 coord;\n"
                                 "varying vec2 texcoord;\n"
                                 "void main(void) {\n"
                                 "    gl_Position = vec4(coord.xy, 0, 1);\n"
                                 "    texcoord = coord.zw;\n"
                                 "}\0";

const char *fragmentShaderSource =
    "#version 120\n"
    "varying vec2 texcoord;\n"
    "uniform sampler2D tex;\n"
    "uniform vec4 color;\n"
    "void main(void) {\n"
    "    gl_FragColor = vec4(1, 1, 1, texture2D(tex, texcoord).r) * color;\n"
    "}\0";

void check_gl_error(const char *function) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "OpenGL error in %s: %d\n", function, err);
  }
}

GLuint compile_shader(GLenum type, const char *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char info_log[512];
    glGetShaderInfoLog(shader, 512, NULL, info_log);
    fprintf(stderr, "Shader compilation error: %s\n", info_log);
  }
  check_gl_error("compile_shader");
  return shader;
}

GLuint create_program(const char *vertex_source, const char *fragment_source) {
  GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_source);
  GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_source);

  GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char info_log[512];
    glGetProgramInfoLog(program, 512, NULL, info_log);
    fprintf(stderr, "Program linking error: %s\n", info_log);
  }
  check_gl_error("create_program");

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return program;
}

void render_text(const char *text, float x, float y, float sx, float sy,
                 FT_Face face, GLuint program, GLuint vbo, GLuint tex) {
  FT_GlyphSlot g = face->glyph;

  glUseProgram(program);
  glUniform1i(glGetUniformLocation(program, "tex"), 0);
  glUniform4f(glGetUniformLocation(program, "color"), 0.5, 0.8, 0.2, 1);
  check_gl_error("render_text: setting uniforms");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  check_gl_error("render_text: binding texture");

  for (const char *p = text; *p; p++) {
    if (FT_Load_Char(face, *p, FT_LOAD_RENDER)) {
      continue;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g->bitmap.width, g->bitmap.rows, 0,
                 GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);
    check_gl_error("render_text: updating texture");

    float x2 = x + g->bitmap_left * sx;
    float y2 = -y - g->bitmap_top * sy;
    float w = g->bitmap.width * sx;
    float h = g->bitmap.rows * sy;

    GLfloat box[4][4] = {{x2, -y2, 0.0f, 0.0f},
                         {x2 + w, -y2, 1.0f, 0.0f},
                         {x2, -y2 - h, 0.0f, 1.0f},
                         {x2 + w, -y2 - h, 1.0f, 1.0f}};

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(box), box);
    check_gl_error("render_text: updating VBO");

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    check_gl_error("render_text: drawing");

    x += (g->advance.x >> 6) * sx;
    y += (g->advance.y >> 6) * sy;
  }
}

int main() {
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }

  GLFWwindow *window =
      glfwCreateWindow(winw, winh, "Text Rendering", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    return -1;
  }

  // Initialize FreeType
  FT_Library ft;
  if (FT_Init_FreeType(&ft)) {
    fprintf(stderr, "Could not init FreeType library\n");
    return -1;
  }

  FT_Face face;
  if (FT_New_Face(ft, "/home/shrestha/.fonts/Meditative.ttf", 0, &face)) {
    fprintf(stderr, "Could not open font\n");
    return -1;
  }
  FT_Set_Pixel_Sizes(face, 0, 48);

  // Create shaders and program
  GLuint program = create_program(vertexShaderSource, fragmentShaderSource);

  // Create texture
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  check_gl_error("creating texture");

  // Create VBO
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * 4, NULL, GL_DYNAMIC_DRAW);
  check_gl_error("creating VBO");

  GLuint attribute_coord = glGetAttribLocation(program, "coord");
  glEnableVertexAttribArray(attribute_coord);
  glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);
  check_gl_error("setting vertex attributes");

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while (!glfwWindowShouldClose(window)) {
    // Update window size
    glfwGetFramebufferSize(window, &winw, &winh);

    // Recalculate scaling factors based on current window size
    float sx = 2.0f / winw; // Width scaling factor
    float sy = 2.0f / winh; // Height scaling factor

    glClear(GL_COLOR_BUFFER_BIT);

    render_text("Hello World", -1 + 8.5 * sx, 1 - 100.5 * sy, sx, sy, face,
                program, vbo, tex);
    render_text("The Quick Brown Fox Jumps Over The Lazy Dog", -1.0f + 8 * sx,
                1.0f - 50 * sy, sx, sy, face, program, vbo, tex);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteTextures(1, &tex);
  glDeleteBuffers(1, &vbo);
  glDeleteProgram(program);
  FT_Done_Face(face);
  FT_Done_FreeType(ft);
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
