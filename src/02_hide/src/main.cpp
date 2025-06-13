#include "enemy.h"
#include "obstacle.h"
#include "player.h"
#include "raylib.h"
#include "world.h"
#include <memory>

#include <sstream>
#include <string>

// TODO: add enemy which cycles through roaming and chasing the player

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(1280, 800, "Hide");
  SetTargetFPS(60);

  // use a world object to manage update and rendering for all game objects
  World world;
  // use smart pointer for better memory management
  world.set_player(std::make_unique<Player>());
  // randomly place n objects in the world
  for (auto &obstacle : get_world_obstacles(100)) {
    world.add_object(std::make_unique<Obstacle>(obstacle));
  }

  // Add some enemies
  Vector2 enemy_pos = {500, 500};
  auto enemy = std::make_unique<Enemy>(enemy_pos);
  std::vector<Vector2> patrol_points = {
      {500, 500}, {800, 500}, {800, 800}, {500, 800}};
  enemy->set_patrol_points(patrol_points);
  world.add_object(std::move(enemy));

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();

    BeginDrawing();
    ClearBackground(BLACK);

    world.update(dt);
    world.draw();

    // draw some debug info
    DrawText(std::to_string(GetFPS()).c_str(), 5, 5, 24, WHITE);
    Vector2 player_position = world.get_player_ptr()->get_position();
    std::ostringstream oss;
    oss << player_position.x << ", " << player_position.y;
    DrawText(oss.str().c_str(), 5, 25, 24, WHITE);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
