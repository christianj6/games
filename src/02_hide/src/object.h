#pragma once
#include "raylib.h"

class GameObject {
  public:
    // special way to intialize
    GameObject(Vector2 starting_position = {0,0}) : position(starting_position) {}
    virtual void draw() = 0;
    virtual void update(float dt) = 0;
  protected:
    Vector2 position;
};
