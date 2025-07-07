#pragma once
#include "raylib.h"

class World {
  public:
    World() = default;
    void update(float, Vector3);
    void draw();
};
