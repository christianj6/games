#include "game.h"
#include "raylib.h"

Game::Game() : world() {
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
}
