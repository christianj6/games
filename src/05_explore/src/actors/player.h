#pragma once
#include "actor.h"
#include "raylib.h"
#include "systems/movement/blink_command.h"
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

  // blink — 0.0 = full stop, e.g. 0.15 = slow motion during hold preview
  const float blink_time_scale_    = 0.0f;
  const float tap_blink_range_     = 18.0f;
  const float hold_blink_range_    = 24.0f;
  const int   tap_threshold_       = 10;   // frames; under = tap, over = hold

  enum class BlinkState { IDLE, HOLDING, PREVIEWING };
  BlinkState blink_state_         = BlinkState::IDLE;
  int  blink_hold_frames_         = 0;
  bool prev_blink_held_           = false;
  Vector3 blink_target_           = {0, 0, 0};
  bool    blink_target_elevated_  = false;
  JumpList jump_list_;

  void handle_blink(const MovementUpdate &update, World *world);
  void do_blink(Vector3 target, bool record = true);

  // strafe tilt
  const float tilt_max_angle_ = 2.0f;
  const float tilt_lerp_rate_ = 8.0f;
  float tilt_current_ = 0.0f;

  // head bob (vertical + lateral)
  const float bob_freq_ = 1.6f;
  const float bob_height_ = 0.06f;
  const float bob_lateral_ = 0.025f;
  const float bob_sprint_scale_ = 1.5f;
  const float bob_fade_rate_ = 8.0f;
  float bob_timer_ = 0.0f;
  float bob_amplitude_ = 0.0f;

  // landing squash / takeoff kick
  const float squash_amount_ = 0.22f;
  const float takeoff_kick_ = 0.05f;
  const float squash_spring_rate_ = 10.0f;
  float squash_offset_ = 0.0f;
  float prev_vertical_velocity_ = 0.0f;

  // breathing
  const float breathe_freq_ = 1.88f;
  const float breathe_height_ = 0.008f;
  const float breathe_fade_rate_ = 3.0f;
  float breathe_timer_ = 0.0f;
  float breathe_amplitude_ = 0.0f;

  // fov
  const float base_fov_ = 60.0f;
  const float sprint_fov_bonus_ = 6.0f;
  const float fov_lerp_rate_ = 6.0f;

  // jumping
  const float gravity_ = -17.0f;
  const float jump_force_ = 22.0f;
  const float fall_multiplier_ = 1.15f;
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
