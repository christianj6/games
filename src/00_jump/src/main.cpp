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
  std::vector<Obstacle> obstacles;

  const int NUM_OBSTACLES = 3;
  for (size_t i = 0; i < NUM_OBSTACLES; i++) {
    obstacles.emplace_back(i);
  }
  bool game_over = false;
  while (!WindowShouldClose()) {
    player.update();
    for (auto &obj : obstacles) {
      obj.update();
      if (player.is_hit(obj)) {
        game_over = true;
      }
    }

    BeginDrawing();
    ClearBackground(BLACK);
    if (!game_over) {
      player.draw();
      for (auto &obj : obstacles) {
        obj.draw();
      }
    } else {
      const char *text = "GAME OVER";
      DrawText(text, GetScreenWidth() / 2 - MeasureText(text, 60) / 2,
               GetScreenHeight() / 2 - 30, 60, RED);
      DrawText("Press ESC to exit",
               GetScreenWidth() / 2 - MeasureText("Press ESC to exit", 20) / 2,
               GetScreenHeight() / 2 + 40, 20, GRAY);
    }
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
