#pragma once
#include "raylib.h"

class Obstacle {
public:
  Obstacle(float, Vector3, Color);
  void draw();

private:
  float height;
  Vector3 position;
  Color color;
};
