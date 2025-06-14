#include "game.h"
#include "raylib.h"

Game::Game() : world(), player() {}

void Game::update() {
  float dt = GetFrameTime();

  player.handle_input();
}

void Game::draw() {
  BeginDrawing();
  ClearBackground(BLACK);

  BeginMode3D(player.get_camera());
  world.draw();
  EndMode3D();

  EndDrawing();
}
