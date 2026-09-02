#pragma once
#include <memory>
#include <vector>

#include "actors/actor.h"
#include "actors/enemy.h"
#include "actors/friend.h"
#include "actors/item.h"
#include "actors/player.h"
#include "hud.h"
#include "terrain/world.h"
#include "utils/graphics/renderer.h"

enum class GameState { RUNNING, PAUSED, WIN, LOSE };

struct GameInfo {
  bool restart = false;      // pause menu: rebuild the world
  bool quit_to_menu = false; // pause menu: back to the main menu
};

class Game {
public:
  Game();
  ~Game();
  GameInfo tick(bool);
  GameState get_current_state() { return current_state; }

private:
  GameState current_state = GameState::RUNNING;
  GameInfo update();
  void draw();
  void toggle_pause();
  void spawn_quest_items();
  void update_quest();
  GameState previous_state;

  Hud hud;
  float last_health_ = 100.0f;      // for regen damage detection
  float time_since_damage_ = 999.0f;
  World world;
  Player player;
  Renderer renderer;
  std::unique_ptr<Friend> friend_;
  std::vector<std::unique_ptr<Item>> items_;
  std::vector<std::unique_ptr<Actor>> actors;
  Blackboard blackboard;
};
