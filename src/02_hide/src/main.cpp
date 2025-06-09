#include "obstacle.h"
#include "player.h"
#include "raylib.h"
#include "world.h"
#include <memory>

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(1280, 800, "Hide");

  // use a world object to manage update and rendering for all game objects
  World world;
  // use smart pointer for better memory management
  world.set_player(std::make_unique<Player>());
  // randomly place n objects in the world
  for (auto &obstacle : get_world_obstacles(100)) {
    world.add_object(std::make_unique<Obstacle>(obstacle));
  }

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();

    BeginDrawing();
    ClearBackground(BLACK);

    world.update(dt);
    world.draw();

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
