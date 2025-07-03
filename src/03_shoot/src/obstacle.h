#pragma once
#include "raylib.h"

class Obstacle {
public:
  Obstacle(float, Vector3, Color);
  void draw();
  Vector3 get_position();
  float get_height();

private:
  float height;
  Vector3 position;
  Color color;
};
