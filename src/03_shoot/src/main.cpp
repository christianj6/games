#include "game.h"
#include "raylib.h"

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI | FLAG_FULLSCREEN_MODE);
  InitWindow(0, 0, "Shoot");
  Game game;

  DisableCursor();
  while (!WindowShouldClose()) {
    game.update();
    game.draw();
  }

  CloseWindow();
  return 0;
}
