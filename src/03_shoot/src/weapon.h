#pragma once
#include "raylib.h"
#include "weapon.h"

class Weapon {
public:
  Weapon(Vector3 pos);
  void draw();
  Vector3 get_position() const { return position; }

private:
  Vector3 position;
  const float RADIUS = 1.0f;
};
