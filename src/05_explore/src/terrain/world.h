#pragma once
#include "raylib.h"

class World {
  public:
    World() = default;
    void update(float, Vector3);
    void draw();
    bool position_is_acceptable(const Vector3);
};
