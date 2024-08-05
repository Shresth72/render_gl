#include "game_app.h"

int send_message_to_server(const char *message) {
  int sockfd;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE];

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    printf("Socket creation failed\n");
    return -1;
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    printf("Connection to server failed\n");
    close(sockfd);
    return -1;
  }

  char message_with_newline[256];
  snprintf(message_with_newline, sizeof(message_with_newline), "%s\n", message);

  if (send(sockfd, message_with_newline, strlen(message_with_newline), 0) < 0) {
    printf("Sending message to the server failed\n");
    close(sockfd);
    return -1;
  }

  memset(buffer, 0, BUFFER_SIZE);
  int bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
  if (bytes_received < 0) {
    printf("Error receiving response from server\n");
    close(sockfd);
    return -1;
  }

  // printf("Server: %s\n", buffer);

  close(sockfd);
  return 0;
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

  float currentR = app->renderer->shader->r;
  if (currentR != app->renderer->shader->prevR) {
    char message[256];
    snprintf(message, sizeof(message), "Shader r value: %f\n",
             app->renderer->shader->r);
    if (send_message_to_server(message) == -1) {
      return QUIT;
    }
    app->renderer->shader->prevR = currentR;
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

  GLCall(glfwWindowHint(GLFW_DECORATED, GLFW_FALSE));

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

// Utils
void calculate_frame_rate(GameApp *app) {
  GLCall(app->currentTime = glfwGetTime());
  app->numFrames++;
  if (app->currentTime - app->lastTime >= 1.0) {
    printf("%f ms/frame\n", 1000.0 / (double)app->numFrames);
    app->numFrames = 0;
    app->lastTime += 1.0;
  }
}

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
