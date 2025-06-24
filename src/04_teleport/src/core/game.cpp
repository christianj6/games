#include "game.h"
#include "raylib.h"
#include <stdlib.h>

Game::Game() : hud(), player(), world() {}

void Game::update() {
  float dt = GetFrameTime();
  player.update(dt);
  world.update(dt, player.get_camera());
}

void Game::draw() {
  BeginDrawing();
  ClearBackground(RAYWHITE);

  BeginMode3D(player.get_camera());
  world.draw();
  EndMode3D();

  hud.draw(player.get_position());
  EndDrawing();
}
