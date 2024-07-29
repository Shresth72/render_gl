#include "game_app.h"

GameApp *game_app_create(GameAppCreateInfo *createInfo) {
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return NULL;
  }

  GameApp *app = (GameApp *)malloc(sizeof(GameApp));
  app->width = createInfo->width;
  app->height = createInfo->height;

  app->window = make_window(app->width, app->height);
  if (!app->window) {
    free(app);
    return NULL;
  }

  app->renderer = engine_create(app->width, app->height);

  app->lastTime = glfwGetTime();
  app->currentTime = app->lastTime;
  app->numFrames = 0;

  return app;
}

returnCode game_app_main_loop(GameApp *app) {
  app->currentTime = glfwGetTime();
  app->numFrames++;
  if (app->currentTime - app->lastTime >= 1.0) {
    printf("%f ms/frame\n", 1000.0 / (double)app->numFrames);
    app->numFrames = 0;
    app->lastTime += 1.0;
  }

  engine_render(app->renderer);
  glfwSwapBuffers(app->window);
  glfwPollEvents();

  if (glfwWindowShouldClose(app->window)) {
    return QUIT;
  }

  return CONTINUE;
}

void game_app_destroy(GameApp *app) {
  engine_destroy(app->renderer);
  glfwDestroyWindow(app->window);
  glfwTerminate();
  free(app);
}

GLFWwindow *make_window(int width, int height) {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(width, height, "Shader", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return NULL;
  }

  glfwMakeContextCurrent(window);
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    return NULL;
  }

  return window;
}
