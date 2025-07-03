#pragma once
#include "actors/enemy.h"
#include "actors/player.h"
#include "hud.h"
#include "world/world.h"
#include <vector>

class Game {
public:
  // game is just passing through the debug bool
  Game(bool);
  ~Game();
  void update();
  void draw();

private:
  Hud hud;
  Player player;
  World world;
  std::vector<Enemy> enemies;
  Map map;
  micropather::MicroPather *pather;
  bool global_player_visibility_flag;
};
