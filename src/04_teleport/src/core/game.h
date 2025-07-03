#pragma once
#include "actors/enemy.h"
#include "actors/player.h"
#include "hud.h"
#include "world/world.h"
#include <vector>

// the game also needs states like game over etc
// in order to encapsulate this and distinguish from app

class Game {
public:
  // game is just passing through the debug bool
  Game(bool);
  ~Game();
  // use the update return as a simple code
  int update();
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
