#pragma once
#include "actor.h"
#include "raylib.h"
#include "systems/movement/controller.h"

class Player : public Actor {
public:
  Player();
  MovementUpdate update(float, Blackboard &) override;
  void draw() override;
  Camera3D get_camera() { return camera; }

private:
  Camera3D camera;
  void setup_camera();
  Vector3 adjust_movement_relative_to_camera(float, Vector3);

  // jumping
  const float gravity_ = -20.0f;
  const float jump_force_ = 14.0f;
  float vertical_velocity_ = 0.0f;
  const int max_jumps_ = 1;
  int jumps_remaining_ = max_jumps_;
  const int jump_buffer_frames_ = 6;
  int jump_buffer_ = 0;
};
