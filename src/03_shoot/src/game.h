#pragma once
#include "hud.h"
#include "player.h"
#include "world.h"

class Game {
public:
  Game();
  void update();
  void draw();

private:
  World world;
  Player player;
  Hud hud;
  bool game_over;
  float game_over_timer;
  static constexpr float CATCH_DISTANCE = 1.5f;
};
