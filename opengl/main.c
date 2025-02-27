#include "control/game_app.h"

int main() {
  int width = 960;
  int height = 540;

  GameAppCreateInfo appInfo;
  appInfo.width = width;
  appInfo.height = height;
  appInfo.fontPath =
      "/home/shres/c_projects/render_gl/opengl/nanovg/fonts/Meditative.ttf";
  GameApp *app = game_app_create(&appInfo);

  returnCode nextAction = CONTINUE;
  while (nextAction == CONTINUE) {
    nextAction = game_app_main_loop(app);
  }

  game_app_destroy(app);

  return 0;
}
