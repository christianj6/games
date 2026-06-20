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
  Vector3 camera_relative_direction(Vector3 input);

  // horizontal movement
  enum class SprintMode { Hold, Toggle };
  const SprintMode sprint_mode_ = SprintMode::Toggle;

  const float max_speed_ = 6.5f;
  const float sprint_speed_ = 11.0f;
  const float accel_rate_ = 18.0f;
  const float decel_rate_ = 20.0f;
  const float air_control_ = 0.15f;
  Vector3 horizontal_velocity_ = {0.0f, 0.0f, 0.0f};
  bool sprint_active_ = false;
  bool prev_sprint_ = false;

  // head bob
  const float bob_freq_ = 1.6f;
  const float bob_height_ = 0.06f;
  const float bob_sprint_scale_ = 1.5f;
  const float bob_fade_rate_ = 8.0f;
  float bob_timer_ = 0.0f;
  float bob_amplitude_ = 0.0f;

  // landing squash
  const float squash_amount_ = 0.22f;
  const float squash_spring_rate_ = 10.0f;
  float squash_offset_ = 0.0f;
  float prev_vertical_velocity_ = 0.0f;

  // fov
  const float base_fov_ = 60.0f;
  const float sprint_fov_bonus_ = 6.0f;
  const float fov_lerp_rate_ = 6.0f;

  // jumping
  const float gravity_ = -20.0f;
  const float jump_force_ = 22.0f;
  const float fall_multiplier_ = 1.4f;
  const float fall_blend_threshold_ = 6.0f;
  const float apex_hang_threshold_ = 5.0f;
  const float apex_hang_reduction_ = 0.4f;
  const float jump_cut_gravity_ = -28.0f;
  float vertical_velocity_ = 0.0f;
  const int max_jumps_ = 2;
  int jumps_remaining_ = max_jumps_;
  const int jump_buffer_frames_ = 6;
  int jump_buffer_ = 0;
  const int coyote_time_frames_ = 8;
  int coyote_frames_ = 0;
};
