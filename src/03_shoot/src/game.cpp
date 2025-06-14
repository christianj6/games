#include "game.h"
#include "raylib.h"

Game::Game() : world(), player(), hud() {}

void Game::update() {
  float dt = GetFrameTime();

  player.handle_input();
  player.update(dt);
  world.update(dt);
  hud.update(dt);
}

void Game::draw() {
  BeginDrawing();
  ClearBackground(BLACK);

  BeginMode3D(player.get_camera());
  world.draw();
  EndMode3D();
  hud.draw();

  EndDrawing();
}
