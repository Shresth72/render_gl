#include "game_app.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  GLCall(GameApp *app = (GameApp *)glfwGetWindowUserPointer(window));
  GLCall(glViewport(0, 0, width, height));
  app->width = width;
  app->height = height;
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  GLCall(GameApp *app = (GameApp *)glfwGetWindowUserPointer(window));
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
    GLCall(glfwTerminate());
    free(app);
    return NULL;
  }

  GLCall(glfwSetWindowUserPointer(app->window, app));
  GLCall(
      glfwSetFramebufferSizeCallback(app->window, framebuffer_size_callback));
  GLCall(glfwSetMouseButtonCallback(app->window, mouse_button_callback));

  glfwSwapInterval(20);

  app->renderer = engine_create(app->width, app->height, createInfo->fontPath);
  if (!app->renderer) {
    GLCall(glfwDestroyWindow(app->window));
    free(app);
    return NULL;
  }

  GLCall(app->lastTime = glfwGetTime());
  app->currentTime = app->lastTime;
  app->numFrames = 0;

  return app;
}

returnCode game_app_main_loop(GameApp *app) {
  calculate_frame_rate(app);

  GLCall(glfwGetCursorPos(app->window, &app->mouseX, &app->mouseY));
  GLCall(glViewport(0, 0, app->width, app->height));
  GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

  engine_render(app->renderer, app->width, app->height, app->mousePressed,
                app->mouseX, app->mouseY);

  GLCall(glfwSwapBuffers(app->window));
  GLCall(glfwPollEvents());

  if (glfwWindowShouldClose(app->window)) {
    return QUIT;
  }

  return CONTINUE;
}

void game_app_destroy(GameApp *app) {
  engine_destroy(app->renderer);
  GLCall(glfwDestroyWindow(app->window));
  GLCall(glfwTerminate());
  free(app);
}

GLFWwindow *make_window(int width, int height) {
  GLCall(glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3));
  GLCall(glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3));
  GLCall(glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE));

  GLCall(glfwWindowHint(GLFW_DECORATED, GLFW_TRUE));

  GLCall(GLFWwindow *window =
             glfwCreateWindow(width, height, "Shader", NULL, NULL));
  if (!window) {
    GLCall(glfwTerminate());
    return NULL;
  }

  GLCall(glfwMakeContextCurrent(window));
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    return NULL;
  }

  return window;
}

void calculate_frame_rate(GameApp *app) {
  GLCall(app->currentTime = glfwGetTime());
  app->numFrames++;
  if (app->currentTime - app->lastTime >= 1.0) {
    printf("%f ms/frame\n", 1000.0 / (double)app->numFrames);
    app->numFrames = 0;
    app->lastTime += 1.0;
  }
}
