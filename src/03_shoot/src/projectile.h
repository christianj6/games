#pragma once
#include "raylib.h"

class Projectile {
public:
  Projectile(Vector3 start_pos, Vector3 direction);
  void update(float dt);
  void draw();
  bool is_active() const { return active; }
  Vector3 get_position() const { return position; }
  void deactivate() { active = false; }

private:
  Vector3 position;
  Vector3 velocity;
  bool active = true;
  float SPEED = 30.0f;
  float RADIUS = 0.2f;
  float MAX_LIFETIME = 3.0f; // seconds
  float lifetime = 0.0f;
};
