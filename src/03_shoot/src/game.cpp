#include "game.h"
#include "raylib.h"

Game::Game() : world(), player(), hud() { player.set_world(&world); }

void Game::update() {
  float dt = GetFrameTime();

  player.handle_input();
  Vector3 current_player_position = player.update(dt);
  world.update(dt, current_player_position);
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
