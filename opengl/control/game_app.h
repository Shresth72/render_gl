#pragma once

#include "../engine/engine.h"
#include "../utils/errors.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
  int width;
  int height;
  const char *fontPath;
} GameAppCreateInfo;

typedef enum { CONTINUE, QUIT } returnCode;

typedef struct {
  GLFWwindow *window;
  Engine *renderer;
  int width;
  int height;

  double lastTime;
  double currentTime;
  int numFrames;

  int mousePressed;
} GameApp;

GameApp *game_app_create(GameAppCreateInfo *createInfo);
returnCode game_app_main_loop(GameApp *app);
void game_app_destroy(GameApp *app);

GLFWwindow *make_window(int width, int height);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void calculate_frame_rate(GameApp *app);
int send_message_to_server(const char *message);

// Interactions
void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods);
void handle_arrow_keys(GameApp *app);
void handle_jump(GameApp *app);
