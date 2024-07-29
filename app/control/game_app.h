#pragma once
#include "../view/engine.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int width;
  int height;
  const char *fontPath;
} GameAppCreateInfo;

typedef enum { CONTINUE, QUIT } returnCode;

typedef struct {
  GLFWwindow *window;
  int width;
  int height;
  Engine *renderer;

  double lastTime;
  double currentTime;
  int numFrames;

  int mousePressed;
  double mouseX, mouseY;
} GameApp;

GameApp *game_app_create(GameAppCreateInfo *createInfo);
returnCode game_app_main_loop(GameApp *app);
void game_app_destroy(GameApp *app);
GLFWwindow *make_window(int width, int height);
void calculate_frame_rate(GameApp *app);
