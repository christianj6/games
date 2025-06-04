#include "obstacle.h"
#include "player.h"
#include "raylib.h"
#include "resource_dir.h"
#include <vector>

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(1280, 800, "Jump");
  SearchAndSetResourceDir("resources");

  Player player = Player();
  std::vector<Obstacle> obstacles(1, Obstacle());

  while (!WindowShouldClose()) {
    player.update();
    for (auto &obj : obstacles) {
      obj.update();
    }

    BeginDrawing();
    ClearBackground(BLACK);
    player.draw();
    for (auto &obj : obstacles) {
      obj.draw();
    }
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
