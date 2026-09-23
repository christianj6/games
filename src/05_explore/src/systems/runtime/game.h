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
  GameInfo tick();
  GameState get_current_state() const { return current_state_; }
  void toggle_pause(); // public: web main loop forwards ESC-exits-lock as pause

private:
  GameState current_state_ = GameState::RUNNING;
  void update();
  void draw();
  void spawn_quest_items();
  void update_quest();
  Vector3 sample_shard_position(const Vector3 &player_spawn,
                                const std::vector<Vector3> &placed);
  void spawn_shard_guards(const Vector3 &shard_pos);
  void update_listener();
  void pair_revive_targets();
  void update_health(float dt);
  GameState previous_state_;

  Hud hud;
  float last_health_ = kMaxPlayerHealth; // for regen damage detection
  float time_since_damage_ = 999.0f;
  World world;
  Player player;
  Renderer renderer;
  std::unique_ptr<Friend> friend_;
  std::vector<std::unique_ptr<Item>> items_;
  std::vector<std::unique_ptr<Actor>> actors;
  Blackboard blackboard;
};
