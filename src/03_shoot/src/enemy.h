#pragma once
#include "raylib.h"

class Enemy {
public:
  Enemy();
  void update(float dt);
  void draw();
  Vector3 get_position() const { return position; }

private:
  Vector3 position;
  float patrol_radius; // Radius of patrol circle
  float patrol_speed;  // Speed of movement in radians/sec
  float patrol_angle;  // Current angle in patrol circle
  float sphere_radius; // Radius of enemy sphere
  Color color;         // Color of enemy sphere
};
