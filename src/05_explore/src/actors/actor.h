#pragma once
#include "raylib.h"
#include "systems/movement/controller.h"
#include "terrain/world.h"

#include <memory>

struct Blackboard {
  Vector3 current_player_position;
};

class Actor {
public:
  Actor() = default;
  virtual void get_nearest_available_position(Vector3); // public?
  virtual MovementUpdate update(float, Blackboard &);
  virtual void draw() = 0;

  void set_world(std::shared_ptr<World>);

protected:
  Vector3 current_position;
  std::shared_ptr<World> world;
  std::unique_ptr<MovementController> movement_controller;
};
