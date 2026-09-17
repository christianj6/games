#pragma once
#include "raylib.h"
#include "systems/movement/controller.h"
#include "terrain/world.h"

#include <memory>

enum class QuestState { COLLECTING, TURN_IN, COMPLETE };

struct QuestInfo {
  QuestState state = QuestState::COLLECTING;
  int required = 0;
  int collected = 0;
};

struct Blackboard {
  Vector3 current_player_position;
  World *world = nullptr;
  float time_scale = 1.0f; // set by player during hold-blink; applied to world/actor dt
  float player_health = 100.0f; // Phase 2: enemy damage; HUD reads it today
  QuestInfo quest;
  bool friend_nearby = false; // set by Friend when player is in talk range
  float damage_flash = 0.0f; // decays in Game::update; HUD draws a red vignette
  bool takedown_available = false; // behind an unaware guard
  bool attack_available = false;   // in range, not from behind
  Vector3 listener_right = {1.0f, 0.0f, 0.0f}; // for audio panning; set by Game
};

class Actor {
public:
  Actor() = default;
  virtual ~Actor() = default;
  virtual MovementUpdate update(float, Blackboard &) = 0;
  virtual void draw() = 0;
  virtual bool is_dead() const { return false; }


protected:
  virtual MovementUpdate get_update(float, Blackboard &);
  Vector3 current_position;
  std::shared_ptr<World> world;
  std::unique_ptr<MovementController> movement_controller;
};
