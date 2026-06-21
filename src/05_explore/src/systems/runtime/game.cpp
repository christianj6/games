#include "game.h"
#include "raylib.h"
#include <memory>

Game::Game() {
  blackboard.world = &world;

  // TODO: remove sequential coupling
  world.set_renderer(&renderer);
  world.build_chunks();
}

GameInfo Game::tick(bool debug) {
  if (current_state == GameState::RUNNING) {
    update();
  }
  if (IsKeyPressed(KEY_ESCAPE) ||
      IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) {
    toggle_pause();
  }
  draw();

  return {};
}

GameInfo Game::update() {
  float dt = GetFrameTime();

  blackboard.current_player_position = player.update(dt, blackboard).position;
  float scaled_dt = dt * blackboard.time_scale;
  world.update(scaled_dt, blackboard.current_player_position);
  renderer.update(player.get_camera());
  for (auto &a : actors) {
    a->update(scaled_dt, blackboard);
  }

  return {};
}

void Game::draw() {
  BeginMode3D(player.get_camera());

  player.draw();
  world.draw();
  for (auto &a : actors) {
    a->draw();
  }
  EndMode3D();

  player.draw_hud(player.get_camera());

  if (current_state == GameState::PAUSED) {
    DrawText("PAUSED", 20, 20, 20, RED);
  }

  hud.draw(blackboard);
}

void Game::toggle_pause() {
  if (current_state != GameState::PAUSED) {
    previous_state = current_state;
    current_state = GameState::PAUSED;
  } else if (current_state == GameState::PAUSED) {
    current_state = previous_state;
  }
}
