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
  bool game_won;
  float game_over_timer;
  // static means it belongs to the class itself (like putting below class name
  // in python) constexpr is a stronger form of const meaning compute at compile
  // time
  static constexpr float CATCH_DISTANCE = 1.5f;
};
