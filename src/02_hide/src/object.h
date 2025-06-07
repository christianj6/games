#pragma once
#include "raylib.h"

class GameObject {
  public:
    // member initialization rather than assignment in constructor body
    GameObject(Vector2 starting_position = {0,0}) : position(starting_position) {}

    virtual void draw() = 0;
    virtual void update(float dt) = 0;

  // protected means that derived classes can access this attribute
  protected:
    Vector2 position;
  
  public:
    Vector2 get_position() const { return position; }
};
