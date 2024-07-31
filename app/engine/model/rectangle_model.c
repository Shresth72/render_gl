#include "rectangle_model.h"

RectangleModel *rectangle_model_create(RectangleModelCreateInfo *createInfo) {
  RectangleModel *model = malloc(sizeof(RectangleModel));
  if (!model) {
    return NULL; // Check for memory allocation failure
  }

  float l = createInfo->size.x / 2;
  float w = createInfo->size.y / 2;
  float h = createInfo->size.z / 2;

  // Initialize vertex data
  float vertices[] = {                        // bottom
                      -l, -w, -h, 0.0f, 0.0f, //
                      l, -w, -h, 1.0f, 0.0f,  //
                      l, w, -h, 1.0f, 1.0f,   //
                      l, w, -h, 1.0f, 1.0f,   //
                      -l, w, -h, 0.0f, 1.0f,  //
                      -l, -w, -h, 0.0f, 0.0f, //
                                              //
                      // top
                      -l, -w, h, 0.0f, 0.0f, //
                      l, -w, h, 1.0f, 0.0f,  //
                      l, w, h, 1.0f, 1.0f,   //
                      l, w, h, 1.0f, 1.0f,   //
                      -l, w, h, 0.0f, 1.0f,  //
                      -l, -w, h, 0.0f, 0.0f, //
                                             //
                      // left
                      -l, w, h, 1.0f, 0.0f,   //
                      -l, w, -h, 1.0f, 1.0f,  //
                      -l, -w, -h, 0.0f, 1.0f, //
                      -l, -w, -h, 0.0f, 1.0f, //
                      -l, -w, h, 0.0f, 0.0f,  //
                      -l, w, h, 1.0f, 0.0f,   //
                                              //
                      // right
                      l, w, h, 1.0f, 0.0f,   //
                      l, w, -h, 1.0f, 1.0f,  //
                      l, -w, -h, 0.0f, 1.0f, //
                      l, -w, -h, 0.0f, 1.0f, //
                      l, -w, h, 0.0f, 0.0f,  //
                      l, w, h, 1.0f, 0.0f,   //
                                             //
                      // back
                      -l, -w, -h, 0.0f, 1.0f, //
                      l, -w, -h, 1.0f, 1.0f,  //
                      l, -w, h, 1.0f, 0.0f,   //
                      l, -w, h, 1.0f, 0.0f,   //
                      -l, -w, h, 0.0f, 0.0f,  //
                      -l, -w, -h, 0.0f, 1.0f, //
                                              //
                      // front
                      -l, w, -h, 0.0f, 1.0f, //
                      l, w, -h, 1.0f, 1.0f,  //
                      l, w, h, 1.0f, 0.0f,   //
                      l, w, h, 1.0f, 0.0f,   //
                      -l, w, h, 0.0f, 0.0f,  //
                      -l, w, -h, 0.0f, 1.0f};

  model->vertexCount = sizeof(vertices) / sizeof(vertices[0]) / 5;
  model->vertices = malloc(model->vertexCount * 5 * sizeof(float));
  if (!model->vertices) {
    free(model);
    return NULL;
  }
  memcpy(model->vertices, vertices, sizeof(vertices));

  // Generate and bind VBO and VAO
  glCreateBuffers(1, &model->VBO);
  glCreateVertexArrays(1, &model->VAO);
  glVertexArrayVertexBuffer(model->VAO, 0, model->VBO, 0, 5 * sizeof(float));
  glNamedBufferStorage(model->VBO, model->vertexCount * 5 * sizeof(float),
                       model->vertices, GL_DYNAMIC_STORAGE_BIT);

  glEnableVertexArrayAttrib(model->VAO, 0);
  glEnableVertexArrayAttrib(model->VAO, 1);
  glVertexArrayAttribFormat(model->VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribFormat(model->VAO, 1, 2, GL_FLOAT, GL_FALSE,
                            3 * sizeof(float));
  glVertexArrayAttribBinding(model->VAO, 0, 0);
  glVertexArrayAttribBinding(model->VAO, 1, 0);

  return model;
}

void rectangle_model_destroy(RectangleModel *model) {
  if (model) {
    glDeleteBuffers(1, &model->VBO);
    glDeleteVertexArrays(1, &model->VAO);
    free(model->vertices);
    free(model);
  }
}
