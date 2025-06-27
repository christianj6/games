#pragma once
#include "actors/player.h"
#include "hud.h"
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
