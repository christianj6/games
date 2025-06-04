#include "player.h"
#include "raylib.h"
#include "resource_dir.h"

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(1280, 800, "Jump");
  SearchAndSetResourceDir("resources");

  Player player = Player();
  while (!WindowShouldClose()) {
    player.update();

    BeginDrawing();
    ClearBackground(BLACK);
    player.draw();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
