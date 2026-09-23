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

// Shared NPC tuning (enemy + friend): the floor-walkability ceiling and the
// half-height any grounded actor floats above the floor.
constexpr float kWalkableFloorMax = 2.5f;
constexpr float kActorHalfHeight = 0.9f;

// Max player health; also the HUD bar length and the regen ceiling.
constexpr float kMaxPlayerHealth = 100.0f;

struct Blackboard {
  Vector3 current_player_position = {0, 0, 0};
  World *world = nullptr;
  float time_scale =
      1.0f; // set by player during hold-blink; applied to world/actor dt
  float player_health = kMaxPlayerHealth; // 0-100; reduced by enemy attacks,
                                          // drives the HUD bar and vignette
  QuestInfo quest;
  bool friend_nearby = false; // set by Friend when player is in talk range
  float damage_flash = 0.0f; // decays in Game::update; HUD draws a red vignette
  bool takedown_available = false;             // behind an unaware guard
  bool attack_available = false;               // in range, not from behind
  Vector3 listener_right = {1.0f, 0.0f, 0.0f}; // for audio panning; set by Game
};

class Renderer;

class Actor {
public:
  Actor() = default;
  virtual ~Actor();
  virtual MovementUpdate update(float dt, Blackboard &blackboard) = 0;
  virtual void draw() = 0;
  virtual bool is_dead() const { return false; }

protected:
  MovementUpdate get_update(float dt, Blackboard &blackboard);
  Vector3 current_position_;
  std::unique_ptr<MovementController> movement_controller_;

  // Staged Kenney-style model; fall back to primitives when unavailable.
  void load_actor_model(const char *path, float target_height,
                        Renderer *renderer);
  void unload_actor_model();
  Model model_{};
  bool model_loaded_ = false;
  float model_scale_ = 1.0f;

  // Shared waypoint bookkeeping for the NPCs' random walkable wander.
  bool try_place_target(World *world, Vector3 candidate);
  bool target_reached(float radius);
  Vector3 target_ = {0, 0, 0};
  bool has_target_ = false;
};
