#pragma once
#include "hud.h"
#include "objects/player.h"
#include "world/world.h"

class Game {
public:
  Game();
  void update();
  void draw();

private:
  Hud hud;
  Player player;
  World world;
};
