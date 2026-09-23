#pragma once
#include "actor.h"
#include "raylib.h"
#include "systems/movement/controller.h"
#include "systems/movement/recall_lists.h"

class Player : public Actor {
public:
  Player();
  MovementUpdate update(float, Blackboard &) override;
  void draw() override;
  void draw_hud(const Camera3D &view_camera);
  const Camera3D &get_camera() const { return camera_; }

private:
  Camera3D camera_;
  const Vector3 spawn_position_ = {20.0f, 3.0f, 20.0f};
  const Vector3 spawn_target_ = {50.0f, 3.0f, 50.0f};
  void setup_camera();
  Vector3 camera_relative_direction(Vector3 input);

  // horizontal movement
  enum class SprintMode { HOLD, TOGGLE };
  const SprintMode sprint_mode_ = SprintMode::TOGGLE;

  const float max_speed_ = 6.5f;
  const float sprint_speed_ = 11.0f;
  const float accel_rate_ = 18.0f;
  const float decel_rate_ = 20.0f;
  const float air_control_ = 0.15f;
  const float eye_height_ = 2.0f;
  const float look_sensitivity_ = 0.095f;
  const float ledge_probe_distance_ = 0.7f;
  const float step_up_max_rise_ = 1.1f;
  const float clamber_max_rise_ground_ = 1.9f;
  const float clamber_max_rise_air_ = 2.6f;
  const float ground_probe_radius_ = 0.3f;
  const float clamber_duration_ = 0.3f;
  const float preview_velocity_damp_ = 0.8f;
  const int recover_max_steps_ = 20;
  const float recover_step_height_ = 0.1f;
  Vector3 horizontal_velocity_ = {0.0f, 0.0f, 0.0f};
  bool sprint_active_ = false;
  bool prev_sprint_ = false;

  // blink — 0.0 = full stop, e.g. 0.15 = slow motion during hold preview
  const float blink_time_scale_ = 0.0f;
  const float tap_blink_range_ = 18.0f;
  const float hold_blink_range_ = 24.0f;
  const float tap_blink_min_speed_ = 0.5f;
  const float blink_elevated_threshold_ = 0.5f;
  const int tap_threshold_ = 10; // frames; under = tap, over = hold

  enum class BlinkState { IDLE, HOLDING, PREVIEWING, RECALLING };
  BlinkState blink_state_ = BlinkState::IDLE;
  int blink_hold_frames_ = 0;
  bool prev_blink_held_ = false;
  Vector3 blink_target_ = {0, 0, 0};
  Vector3 blink_target_smooth_ = {0, 0, 0}; // lerped display position
  bool blink_target_elevated_ = false;
  // Ledge clamber
  bool clambering_ = false;
  float clamber_t_ = 0.0f;
  Vector3 clamber_start_ = {0, 0, 0};
  Vector3 clamber_end_ = {0, 0, 0};
  JumpList jump_list_;

  // Recall mode (Phase 2)
  const int recall_threshold_ =
      10; // frames; under = quick tap, over = recall mode
  const float stick_deadzone_ = 0.15f;
  const float wheel_select_magnitude_ = 0.25f;
  const float anchor_flash_decay_rate_ = 4.0f;
  const float blink_flash_decay_rate_ = 14.0f;
  AnchorList anchor_list_;
  int selected_anchor_ = -1; // -1 = nothing selected
  int recall_hold_frames_ = 0;
  bool prev_recall_held_ = false;
  float anchor_place_flash_ = 0.0f;
  float blink_flash_ = 0.0f;  // screen-edge vignette on any blink/recall
  bool chord_active_ = false; // true until both LB+RB released after chord

  // sfx
  const float anchor_place_volume_ = 0.6f;
  const float recall_volume_ = 0.6f;
  const float blink_volume_ = 0.45f;
  const float step_volume_ = 0.4f;

  void handle_blink(const MovementUpdate &frame, World *world);
  void update_anchor_selection();
  void update_blink_hold(const MovementUpdate &frame, World *world);
  void do_blink(Vector3 target, bool record = true);

  // per-frame seams of update()
  bool advance_clamber(const MovementUpdate &frame, float dt);
  void update_vertical(const MovementUpdate &frame, World *world, float floor_y,
                       bool on_ground, float dt);
  void update_horizontal(const MovementUpdate &frame, World *world,
                         float floor_y, bool on_ground, float dt);
  void recover_from_overlap(World *world);
  void update_strafe_tilt(Vector3 look_dir, float dt);
  void update_head_bob(float h_speed, Vector3 cam_right, bool on_ground,
                       float dt);
  void update_breathing(float h_speed, float dt);
  void update_landing_and_fov(float dt);

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
  int prev_step_phase_ = 0;
  float bob_amplitude_ = 0.0f;

  // landing squash / takeoff kick
  const float squash_amount_ = 0.34f;
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

  // hud
  const int blink_vignette_alpha_ = 160;
  const int blink_vignette_thickness_ = 50; // edge thickness in pixels
  const int anchor_flash_alpha_ = 100;
  const float edge_indicator_margin_ = 45.0f;
  // on-screen puck at the anchor's screen position
  const int puck_selected_alpha_ = 220;
  const int puck_unselected_alpha_ = 160;
  const float puck_selected_radius_ = 9.0f;
  const float puck_unselected_radius_ = 6.0f;
  // clamped screen-edge indicator for off-screen anchors
  const int indicator_selected_alpha_ = 230;
  const int indicator_unselected_alpha_ = 180;
  const float indicator_selected_radius_ = 10.0f;
  const float indicator_unselected_radius_ = 7.0f;
  const float indicator_arrow_selected_radius_ = 5.0f;
  const float indicator_arrow_unselected_radius_ = 3.5f;
  const float indicator_arrow_offset_ = 14.0f;

  // world-space blink/anchor markers
  const int blink_marker_alpha_ = 120;
  const float blink_marker_radius_ = 0.5f;
  const float blink_marker_height_ = 1.2f;
  const int anchor_marker_selected_alpha_ = 230;
  const int anchor_marker_unselected_alpha_ = 160;
  const float anchor_marker_selected_radius_ = 0.65f;
  const float anchor_marker_unselected_radius_ = 0.4f;
  const int anchor_stem_alpha_ = 70;
  const float anchor_stem_radius_ = 0.04f;
  const float anchor_stem_height_ = 1.5f;

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
