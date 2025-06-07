#include "obstacle.h"
#include "player.h"
#include "raylib.h"
#include "world.h"

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(1280, 800, "Hide");

  // use a world object to manage update and rendering for all game objects
  World world;
  // use smart pointer for better memory management
  world.set_player(std::make_unique<Player>());

  // Add 2 obstacles on screen
  world.add_object(std::make_unique<Obstacle>(Vector2{400, 300}));
  world.add_object(std::make_unique<Obstacle>(Vector2{800, 500}));

  // Add 3 obstacles off screen
  world.add_object(std::make_unique<Obstacle>(Vector2{1400, 400})); // right
  world.add_object(std::make_unique<Obstacle>(Vector2{-200, 600})); // left
  world.add_object(std::make_unique<Obstacle>(Vector2{600, -100})); // top

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
