#pragma once
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
  // TODO: consider adding a screen member
};
