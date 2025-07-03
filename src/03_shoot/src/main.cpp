#include "game.h"
#include "raylib.h"

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  SetTargetFPS(60);
  InitWindow(2460, 1440, "Shoot");
  DisableCursor();

  Game game;
  DisableCursor();

  while (!WindowShouldClose()) {
    game.update();
    game.draw();
  }

  CloseWindow();
  return 0;
}
