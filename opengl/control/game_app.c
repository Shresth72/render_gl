#include "game_app.h"

GameApp *game_app_create(GameAppCreateInfo *createInfo) {
  GameApp *app = (GameApp *)malloc(sizeof(GameApp));
  app->width = createInfo->width;
  app->height = createInfo->height;

  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    free(app);
    return NULL;
  }

  app->window = make_window(app->width, app->height);
  if (!app->window) {
    glfwTerminate();
    free(app);
    return NULL;
  }

  app->renderer = engine_create(app->width, app->height);
  if (!app->renderer) {
    glfwDestroyWindow(app->window);
    free(app);
    return NULL;
  }

  app->lastTime = glfwGetTime();
  app->currentTime = app->lastTime;
  app->numFrames = 0;

  return app;
}

returnCode game_app_main_loop(GameApp *app) {
  calculate_frame_rate(app);

  engine_render(app->renderer, app->width, app->height);
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

void calculate_frame_rate(GameApp *app) {
  app->currentTime = glfwGetTime();
  app->numFrames++;
  if (app->currentTime - app->lastTime >= 1.0) {
    printf("%f ms/frame\n", 1000.0 / (double)app->numFrames);
    app->numFrames = 0;
    app->lastTime += 1.0;
  }
}
