#define NANOVG_GL3_IMPLEMENTATION
#include "engine.h"
#include "quadmesh.h"

Engine *engine_create(int width, int height, const char *fontPath) {
  Engine *engine = (Engine *)malloc(sizeof(Engine));

  engine->shader =
      util_load_shader("shaders/vertex.glsl", "shaders/fragment.glsl");
  if (engine->shader == 0) {
    free(engine);
    return NULL;
  }

  engine->vg = initializeNanoVG(fontPath);
  if (engine->vg == NULL) {
    printf("Could not init nanovg.\n");
    free(engine);
    return NULL;
  }

  glUniform1f(glGetUniformLocation(engine->shader, "basicTexture"), 0);
  float aspectRatio = (float)width / (float)height;
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  engine_create_models(engine);
  configure_uniform_block(engine);

  return engine;
}

void engine_destroy(Engine *engine) {
  quadmesh_destroy(engine->ourQuad);
  glDeleteProgram(engine->shader);
  nvgDeleteGL3(engine->vg);
  free(engine);
}

void engine_create_models(Engine *engine) {
  engine->ourQuad = quadmesh_create(0.75f, 0.75f);
}

void configure_uniform_block(Engine *engine) {
  engine->blockIndex = glGetUniformBlockIndex(engine->shader, "diskParameters");

  printf("diskParameters is a uniform block occupying block index: %d\n",
         engine->blockIndex);

  glGetActiveUniformBlockiv(engine->shader, engine->blockIndex,
                            GL_UNIFORM_BLOCK_DATA_SIZE, &engine->blockSize);

  printf("\ttaking up %d bytes\n", engine->blockSize);

  GLubyte *blockBuffer = (GLubyte *)malloc(engine->blockSize);

  const GLchar *names[] = {"InnerColor", "OuterColor", "InnerRadius",
                           "OuterRadius"};

  GLuint indices[4];
  glGetUniformIndices(engine->shader, 4, names, indices);

  for (int i = 0; i < 4; ++i) {
    printf("attribute \"%s\" has index: %d in the block.\n", names[i],
           indices[i]);
  }

  GLint offset[4];
  glGetActiveUniformsiv(engine->shader, 4, indices, GL_UNIFORM_OFFSET, offset);

  for (int i = 0; i < 4; ++i) {
    printf("attribute \"%s\" has offset: %d in the block.\n", names[i],
           offset[i]);
  }

  float outerColor[4] = {0.0f, 1.0f, 0.0f, 1.0f};
  float innerColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  float innerRadius = 0.25f;
  float outerRadius = 0.5f;

  memcpy(blockBuffer + offset[0], innerColor, 4 * sizeof(float));
  memcpy(blockBuffer + offset[1], outerColor, 4 * sizeof(float));
  memcpy(blockBuffer + offset[2], &innerRadius, sizeof(float));
  memcpy(blockBuffer + offset[3], &outerRadius, sizeof(float));

  glGenBuffers(1, &engine->UBO);
  glBindBuffer(GL_UNIFORM_BUFFER, engine->UBO);
  glBufferData(GL_UNIFORM_BUFFER, engine->blockSize, blockBuffer,
               GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, engine->blockIndex, engine->UBO);

  free(blockBuffer);
}

//
//
//
// ENGINE RENDER
//
//
//

void engine_render(Engine *engine, int width, int height, int mousePressed,
                   double mouseX, double mouseY) {
  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT);

  engine_shader_render(engine);
  engine_text_render(engine, width, height, mousePressed, mouseX, mouseY);
}

void engine_shader_render(Engine *engine) {
  glUseProgram(engine->shader);
  glBindBufferRange(GL_UNIFORM_BUFFER, engine->blockIndex, engine->UBO, 0,
                    engine->blockSize);
  quadmesh_render(engine->ourQuad);
}

int isPointInRect(float px, float py, float x, float y, float w, float h) {
  return px >= x && px <= x + w && py >= y && py <= y + h;
}

void engine_text_render(Engine *engine, int width, int height, int mousePressed,
                        double mouseX, double mouseY) {
  nvgBeginFrame(engine->vg, width, height, 1.0f);

  // Draw Text
  nvgFontFace(engine->vg, "sans");
  nvgFontSize(engine->vg, 48.0f);
  nvgFillColor(engine->vg, nvgRGBA(255, 255, 255, 255));
  nvgTextAlign(engine->vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
  nvgText(engine->vg, 10.0f, 8.0f, "Hello, NanoVG!", NULL);

  // Draw Button
  float btnW = 120, btnH = 30;
  float btnX = (width - btnW) / 2, btnY = (height - btnH) / 2;
  nvgBeginPath(engine->vg);
  nvgRoundedRect(engine->vg, btnX, btnY, btnW, btnH, 10.f);
  nvgFillColor(engine->vg, nvgRGB(0, 128, 255));
  nvgFill(engine->vg);
  nvgStrokeColor(engine->vg, nvgRGB(0, 0, 0));
  nvgStroke(engine->vg);

  nvgFontSize(engine->vg, 20.0f);
  nvgFillColor(engine->vg, nvgRGBA(255, 255, 255, 255));
  nvgTextAlign(engine->vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
  nvgText(engine->vg, btnX + btnW / 2, btnY + btnH / 2, "Close", NULL);

  if (mousePressed &&
      isPointInRect((float)mouseX, (float)mouseY, btnX, btnY, btnW, btnH)) {
    glfwSetWindowShouldClose(glfwGetCurrentContext(), 1);
  }

  nvgEndFrame(engine->vg);
}

NVGcontext *initializeNanoVG(const char *fontPath) {
  NVGcontext *vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
  if (vg == NULL) {
    return NULL;
  }

  int font = nvgCreateFont(vg, "sans", fontPath);
  if (font == -1) {
    printf("Could not add font.\n");
    nvgDeleteGL3(vg);
    return NULL;
  }

  return vg;
}
