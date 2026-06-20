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
  const float jump_force_ = 18.0f;
  const float fall_multiplier_ = 1.4f;
  const float fall_blend_threshold_ = 6.0f;
  const float apex_hang_threshold_ = 5.0f;
  const float apex_hang_reduction_ = 0.4f;
  const float jump_cut_gravity_ = -28.0f;
  float vertical_velocity_ = 0.0f;
  const int max_jumps_ = 1;
  int jumps_remaining_ = max_jumps_;
  const int jump_buffer_frames_ = 6;
  int jump_buffer_ = 0;
  const int coyote_time_frames_ = 8;
  int coyote_frames_ = 0;
};
