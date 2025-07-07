#pragma once
#include <memory>
#include <vector>

#include "hud.h"
#include "terrain/world.h"
#include "actors/actor.h"
#include "actors/player.h"

enum class GameState { RUNNING, PAUSED, WIN, LOSE };

struct GameInfo {};

class Game {
public:
  Game();
  ~Game() = default;
  GameInfo tick(bool);
  GameState get_current_state() { return current_state; }

private:
  GameState current_state = GameState::RUNNING;
  GameInfo update();
  void draw();
  void toggle_pause();
  GameState previous_state;

  Hud hud;
  World world;
  Player player;
  std::vector<std::unique_ptr<Actor>> actors;
  Blackboard blackboard;
};
