#pragma once
#include "raylib.h"
#include "terrain/world.h"

class Actor {
public:
  Actor() = default;
  virtual void move_to_nearest_available_position(Vector3);
  void set_world(World *);

private:
  Vector3 current_position;
  World *world;
  void draw();
};
