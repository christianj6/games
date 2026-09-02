#pragma once
#include "raylib.h"
#include "systems/movement/controller.h"
#include "terrain/world.h"

#include <memory>

struct Blackboard {
  Vector3 current_player_position;
  World *world = nullptr;
  float time_scale = 1.0f; // set by player during hold-blink; applied to world/actor dt
};

class Actor {
public:
  Actor() = default;
  virtual ~Actor() = default;
  virtual void get_nearest_available_position(Vector3); // public?
  virtual MovementUpdate update(float, Blackboard &) = 0;
  virtual void draw() = 0;

  void set_world(std::shared_ptr<World>);

protected:
  virtual MovementUpdate get_update(float, Blackboard &);
  Vector3 current_position;
  std::shared_ptr<World> world;
  std::unique_ptr<MovementController> movement_controller;
};
