#pragma once
#include "hud.h"
#include "objects/player.h"
#include "world/world.h"

class Game {
public:
  // game is just passing through the debug bool
  Game(bool);
  void update();
  void draw();

private:
  Hud hud;
  Player player;
  World world;
};
