#include "game_app.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  GameApp *app = (GameApp *)glfwGetWindowUserPointer(window);
  glViewport(0, 0, width, height);
  app->width = width;
  app->height = height;
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  GameApp *app = (GameApp *)glfwGetWindowUserPointer(window);
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    app->mousePressed = 1;
  } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    app->mousePressed = 0;
  }
}

GameApp *game_app_create(GameAppCreateInfo *createInfo) {
  GameApp *app = (GameApp *)malloc(sizeof(GameApp));
  app->width = createInfo->width;
  app->height = createInfo->height;
  app->mousePressed = 0;

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

  glfwSetWindowUserPointer(app->window, app);
  glfwSetFramebufferSizeCallback(app->window, framebuffer_size_callback);
  glfwSetMouseButtonCallback(app->window, mouse_button_callback);

  app->renderer = engine_create(createInfo->fontPath);
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

  glfwGetCursorPos(app->window, &app->mouseX, &app->mouseY);
  engine_render(app->renderer, app->width, app->height, app->mousePressed,
                app->mouseX, app->mouseY);
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
