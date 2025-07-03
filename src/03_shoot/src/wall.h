#pragma once
#include "raylib.h"

class Wall {
public:
  Wall(float height, Vector3 position, Color color, float length,
       bool rotate90 = false);
  void draw();

public: // Made public for collision detection
  Vector3 position;
  float height;
  float length;
  Color color;
  bool rotate90; // true if wall should be rotated 90 degrees
};
