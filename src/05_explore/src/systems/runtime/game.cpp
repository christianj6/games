#include "game.h"
#include "raylib.h"

Game::Game() : world() {}

GameInfo Game::tick(bool debug) {
  if (current_state == GameState::RUNNING) {
    update();
  }
  if (IsKeyPressed(KEY_ESCAPE)) {
    toggle_pause();
  }
  draw();

  return {};
}

GameInfo Game::update() {
  float dt = GetFrameTime();

  // TODO: sketch game loop
  // blackboard.current_player_position = player.update(dt);
  // world.update(dt, blackboard);
  // for (auto& a : actors) {
  //  a->update(dt, blackboard);
  // }

  return {};
}

void Game::draw() {
  DrawText("GAME", GetScreenWidth() / 2, GetScreenHeight() / 2, 20, BLACK);
  if (current_state == GameState::PAUSED) {
    DrawText("PAUSED", 20, 20, 20, RED);
  }

  // BeginMode3D(player.get_camera());
  // world.draw();
  // player.draw(world.get_voxel_space_data());
  // for (auto &enemy : enemies) {
  //   enemy.draw();
  // }
  // EndMode3D();
  //
  // hud.draw(player.get_position(), player.check_health());
}

void Game::toggle_pause() {
  if (current_state != GameState::PAUSED) {
    previous_state = current_state;
    current_state = GameState::PAUSED;
  } else if (current_state == GameState::PAUSED) {
    current_state = previous_state;
  }
}
