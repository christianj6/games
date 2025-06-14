#include "enemy.h"
#include "obstacle.h"
#include "player.h"
#include "raylib.h"
#include "world.h"
#include <memory>

// TODO: balance, tidy codebase, finish

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI | FLAG_FULLSCREEN_MODE);
  InitWindow(0, 0, "Hide");
  SetTargetFPS(60);

  // use a world object to manage update and rendering for all game objects
  World world;
  // use smart pointer for better memory management
  world.set_player(std::make_unique<Player>());
  // randomly place n objects in the world
  for (auto &obstacle : get_world_obstacles(100)) {
    world.add_object(std::make_unique<Obstacle>(obstacle));
  }

  Vector2 enemy_pos = {500, 500};
  auto enemy =
      std::make_unique<Enemy>(enemy_pos, 5); // Create with 5 patrol points
  world.add_object(std::move(enemy));

  bool gameover;
  while (!WindowShouldClose()) {
    float dt = GetFrameTime();

    BeginDrawing();
    ClearBackground(BLACK);

    if (!gameover) {
      gameover = !world.update(dt);
      world.draw();
    } else {
      DrawText("loser", GetScreenWidth() / 2, GetScreenHeight() / 2, 50, RED);
      DrawText("press ENTER to restart", GetScreenWidth() / 2,
               (GetScreenHeight() / 2) + 60, 20, GRAY);
      if (IsKeyPressed(KEY_ENTER)) {
        gameover = false;
        world.get_player_ptr()->set_position({0, 0});
      }
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
