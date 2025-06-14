#pragma once
#include "raylib.h"

class GameObject {
  public:
    // member initialization rather than assignment in constructor body
    GameObject(Vector2 starting_position = {0,0}) : position(starting_position) {}

    // abstract methods
    virtual void draw() = 0;
    // declarations don't need variable names especially if abstract
    virtual void update(float, bool = true) = 0;

  // protected means that derived classes can access this attribute
  protected:
    Vector2 position;
  
  public:
    Vector2 get_position() const { return position; }
};
