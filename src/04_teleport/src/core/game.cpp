#include "game.h"
#include "raylib.h"
#include <stdlib.h>

Game::Game(bool debug_mode_enabled) : hud(debug_mode_enabled), world() {}

void Game::update() {
  float dt = GetFrameTime();
  player.update(dt, world.get_voxel_space_data());
  world.update(dt, player.get_camera());
  hud.update(dt);
}

void Game::draw() {
  BeginDrawing();
  ClearBackground(BLACK);

  BeginMode3D(player.get_camera());
  world.draw();
  player.draw();
  EndMode3D();

  hud.draw(player.get_position());
  EndDrawing();
}
