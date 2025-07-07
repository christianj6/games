#pragma once
#include "raylib.h"
#include "terrain/world.h"

#include <memory>

struct Blackboard {
  Vector3 current_player_position;
};

class Actor {
public:
  Actor() = default;
  virtual void move_to_nearest_available_position(Vector3);
  virtual Vector3 update(float, Blackboard) = 0;
  virtual void draw() = 0;

  void set_world(std::shared_ptr<World>);

protected:
  Vector3 current_position;
  std::shared_ptr<World> world;
};
