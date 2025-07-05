#pragma once
#include "raylib.h"
#include "map/world.h"

class Actor {
  public:
    // all actors are aware of the world
    Actor(World*);
    virtual void move_to_nearest_available_position(Vector3);
  private:
    Vector3 current_position;
    World* world;
    void draw();
};
