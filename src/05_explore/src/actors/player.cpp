#include "player.h"
#include "raymath.h"
#include "systems/runtime/audio.h"
#include <cmath>

Player::Player() {
  setup_camera();
  movement_controller_ = std::make_unique<UserMovementController>();
}

void Player::setup_camera() {
  current_position_ = spawn_position_;
  camera_.position = spawn_position_;
  camera_.target = spawn_target_;
  camera_.up = Vector3{0.0f, 1.0f, 0.0f};
  camera_.fovy = base_fov_;
  camera_.projection = CAMERA_PERSPECTIVE;
}

Vector3 Player::camera_relative_direction(Vector3 input) {
  Vector3 forward = {camera_.target.x - camera_.position.x, 0.0f,
                     camera_.target.z - camera_.position.z};
  forward = Vector3Normalize(forward);
  Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera_.up));
  return {right.x * input.x + forward.x * input.z, 0.0f,
          right.z * input.x + forward.z * input.z};
}

void Player::do_blink(Vector3 target, bool record) {
  blink_flash_ = 1.0f; // fire vignette on every blink including recalls
  if (record)
    jump_list_.push({current_position_, target});
  Vector3 view_offset = Vector3Subtract(camera_.target, camera_.position);
  current_position_ = target;
  camera_.position = target;
  camera_.target = Vector3Add(target, view_offset);
  vertical_velocity_ = 0.0f;
}

void Player::handle_blink(const MovementUpdate &frame, World *world) {
  // ── CHORD: LB+RB — place anchor, clear both button states ────────────
  if (frame.place_anchor && !chord_active_) {
    anchor_list_.place(current_position_);
    Audio::get().play(Sfx::Anchor, anchor_place_volume_);
    anchor_place_flash_ = 1.0f;
    chord_active_ = true;
    blink_state_ = BlinkState::IDLE; // cancel any in-progress blink
    blink_hold_frames_ = 0;
    recall_hold_frames_ = 0;
    return;
  }

  // Suppress all LB/RB actions until both buttons are fully released
  if (chord_active_) {
    prev_blink_held_ = frame.blink_held;
    prev_recall_held_ = frame.recall_held;
    blink_hold_frames_ = 0;
    recall_hold_frames_ = 0;
    if (!frame.blink_held && !frame.recall_held) {
      chord_active_ = false;
      blink_state_ = BlinkState::IDLE; // ensure clean state on exit
    }
    return;
  }

  // ── LB STATE MACHINE ─────────────────────────────────────────────────
  bool lb = frame.recall_held;
  bool lb_pressed = lb && !prev_recall_held_;
  bool lb_released = !lb && prev_recall_held_;
  prev_recall_held_ = lb;

  if (lb_pressed)
    recall_hold_frames_ = 0;
  if (lb) {
    recall_hold_frames_++;
    if (recall_hold_frames_ >= recall_threshold_ &&
        blink_state_ == BlinkState::IDLE && !anchor_list_.empty()) {
      blink_state_ = BlinkState::RECALLING;
      selected_anchor_ = -1;
    }
  }
  if (lb_released) {
    if (blink_state_ == BlinkState::RECALLING) {
      if (selected_anchor_ >= 0 && selected_anchor_ < anchor_list_.size())
        do_blink(anchor_list_.get(selected_anchor_), false);
      blink_state_ = BlinkState::IDLE;
    } else if (recall_hold_frames_ < recall_threshold_) {
      // Quick tap: recall most recent blink position
      if (!jump_list_.empty()) {
        Audio::get().play(Sfx::Recall, recall_volume_);
        do_blink(jump_list_.pop().from, false);
      }
    }
    recall_hold_frames_ = 0;
  }

  update_anchor_selection();

  // ── B: cancel PREVIEWING or RECALLING ────────────────────────────────
  if ((blink_state_ == BlinkState::PREVIEWING ||
       blink_state_ == BlinkState::RECALLING) &&
      IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
    blink_state_ = BlinkState::IDLE;
    blink_hold_frames_ = 0;
    recall_hold_frames_ = 0;
    selected_anchor_ = -1;
  }

  update_blink_hold(frame, world);
}

// ── RECALLING: flick right stick to select anchor ─────────────────
void Player::update_anchor_selection() {
  if (blink_state_ != BlinkState::RECALLING)
    return;

  // Keyboard: number keys pick an anchor directly.
  const int anchor_keys[5] = {KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE};
  for (int i = 0; i < anchor_list_.size() && i < 5; ++i) {
    if (IsKeyPressed(anchor_keys[i]))
      selected_anchor_ = i;
  }

  float sx =
      fabsf(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X)) > stick_deadzone_
          ? GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X)
          : 0.0f;
  float sy =
      fabsf(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y)) > stick_deadzone_
          ? GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y)
          : 0.0f;
  float mag = sqrtf(sx * sx + sy * sy);

  // Continuous radial wheel — updates every frame the stick is past the
  // deadzone, keyed to screen-space so indicator position = selection
  // direction.
  if (mag > wheel_select_magnitude_) {
    float fx = sx / mag;
    float fy = -sy / mag; // stick Y inverted: push up = screen up = +fy
    float flick_angle = atan2f(fx, fy);

    Vector2 center = {GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f};
    Vector3 cam_fwd_n =
        Vector3Normalize(Vector3Subtract(camera_.target, camera_.position));

    int best_i = 0;
    float best_diff = 2.0f * PI;
    for (int i = 0; i < anchor_list_.size(); i++) {
      Vector3 pos = anchor_list_.get(i);

      // Project anchor to screen — same math used by the indicator
      Vector2 sp = GetWorldToScreen(pos, camera_);
      bool in_front = Vector3DotProduct(Vector3Subtract(pos, camera_.position),
                                        cam_fwd_n) > 0.0f;

      Vector2 dir = in_front ? Vector2{sp.x - center.x, sp.y - center.y}
                             : Vector2{center.x - sp.x, center.y - sp.y};
      float dlen = sqrtf(dir.x * dir.x + dir.y * dir.y);
      if (dlen < 0.01f)
        continue;

      // screen right = +x, screen up = -y → match stick convention
      float adx = dir.x / dlen;
      float ady = -dir.y / dlen; // invert Y: screen up maps to +fy
      float anchor_angle = atan2f(adx, ady);

      float diff = fabsf(flick_angle - anchor_angle);
      if (diff > PI)
        diff = 2.0f * PI - diff;
      if (diff < best_diff) {
        best_diff = diff;
        best_i = i;
      }
    }
    selected_anchor_ = best_i;
  }
}

// ── RB STATE MACHINE — blocked during recall ──────────────────────────
void Player::update_blink_hold(const MovementUpdate &frame, World *world) {
  if (blink_state_ == BlinkState::RECALLING)
    return;

  bool held = frame.blink_held;
  bool just_released = !held && prev_blink_held_;
  bool just_pressed = held && !prev_blink_held_;
  prev_blink_held_ = held;

  if (just_pressed) {
    blink_state_ = BlinkState::HOLDING;
    blink_hold_frames_ = 0;
  }

  if (held) {
    if (blink_state_ == BlinkState::HOLDING) {
      blink_hold_frames_++;
      if (blink_hold_frames_ >= tap_threshold_)
        blink_state_ = BlinkState::PREVIEWING;
    }
    if (blink_state_ == BlinkState::PREVIEWING) {
      Vector3 dir =
          Vector3Normalize(Vector3Subtract(camera_.target, camera_.position));
      blink_target_ =
          world->find_blink_target(current_position_, dir, hold_blink_range_);
      float origin_floor =
          world->get_floor_height(current_position_.x, current_position_.z);
      float target_floor =
          world->get_floor_height(blink_target_.x, blink_target_.z);
      blink_target_elevated_ =
          (target_floor > origin_floor + blink_elevated_threshold_);
    }
  }

  if (just_released) {
    Audio::get().play(Sfx::Blink, blink_volume_);
    if (blink_state_ == BlinkState::HOLDING) {
      float h_spd = sqrtf(horizontal_velocity_.x * horizontal_velocity_.x +
                          horizontal_velocity_.z * horizontal_velocity_.z);
      Vector3 tap_dir;
      if (h_spd > tap_blink_min_speed_) {
        tap_dir = Vector3Normalize(
            {horizontal_velocity_.x, 0.0f, horizontal_velocity_.z});
      } else {
        Vector3 fwd = Vector3Subtract(camera_.target, camera_.position);
        tap_dir = Vector3Normalize({fwd.x, 0.0f, fwd.z});
      }
      do_blink(world->find_blink_target_through(current_position_, tap_dir,
                                                tap_blink_range_));
    } else if (blink_state_ == BlinkState::PREVIEWING) {
      do_blink(blink_target_);
    }
    blink_state_ = BlinkState::IDLE;
    blink_hold_frames_ = 0;
  }
}

MovementUpdate Player::update(float dt, Blackboard &blackboard) {
  World *world = blackboard.world;
  MovementUpdate frame = Actor::get_update(dt, blackboard);

  // Time dilation: hold-blink scales world/actor dt via blackboard
  blackboard.time_scale = frame.blink_held ? blink_time_scale_ : 1.0f;

  // Vertical
  float floor_y =
      world->get_floor_height(current_position_.x, current_position_.z) +
      eye_height_;
  bool on_ground = current_position_.y <= floor_y && vertical_velocity_ <= 0.0f;

  if (advance_clamber(frame, dt)) {
    // Camera-only frame during the mantle arc.
    return {current_position_,
            {camera_.position.x, camera_.position.y},
            frame.jump,
            false,
            frame.jump_held};
  }

  if (blink_state_ == BlinkState::PREVIEWING ||
      blink_state_ == BlinkState::RECALLING) {
    // Suspend the player in place during preview and recall
    vertical_velocity_ = 0.0f;
    current_position_.y = std::max(current_position_.y, floor_y);
    on_ground = true;
  } else {
    update_vertical(frame, world, floor_y, on_ground, dt);
  }

  prev_vertical_velocity_ = vertical_velocity_;
  squash_offset_ += (0.0f - squash_offset_) * squash_spring_rate_ * dt;

  // Horizontal (blocked during preview and recall)
  update_horizontal(frame, world, floor_y, on_ground, dt);

  // Recovery: if stuck inside geometry push upward
  recover_from_overlap(world);

  // Camera sync
  Vector3 look_dir = Vector3Subtract(camera_.target, camera_.position);
  camera_.position = current_position_;
  camera_.target = Vector3Add(camera_.position, look_dir);

  // Strafe tilt
  update_strafe_tilt(look_dir, dt);

  bool suppress_look = (blink_state_ == BlinkState::RECALLING);
  UpdateCameraPro(&camera_, Vector3{0},
                  suppress_look
                      ? Vector3{0, 0, 0}
                      : Vector3{frame.camera.x * look_sensitivity_,
                                frame.camera.y * look_sensitivity_, 0.0f},
                  0.0f);

  // Blink state machine (uses camera direction after mouse look)
  handle_blink(frame, world);

  // Smooth blink target: lerp display position so the ball never jumps
  if (blink_state_ == BlinkState::PREVIEWING) {
    float t = fminf(15.0f * dt, 1.0f);
    blink_target_smooth_.x += (blink_target_.x - blink_target_smooth_.x) * t;
    blink_target_smooth_.y += (blink_target_.y - blink_target_smooth_.y) * t;
    blink_target_smooth_.z += (blink_target_.z - blink_target_smooth_.z) * t;
  } else {
    blink_target_smooth_ = blink_target_; // snap when not previewing
  }

  // Decay flashes
  if (anchor_place_flash_ > 0.0f)
    anchor_place_flash_ =
        std::max(0.0f, anchor_place_flash_ - dt * anchor_flash_decay_rate_);
  if (blink_flash_ > 0.0f)
    blink_flash_ = std::max(0.0f, blink_flash_ - dt * blink_flash_decay_rate_);
  // ~70ms

  // Kill bob and breathe during recall — camera must be still for selection
  if (blink_state_ == BlinkState::RECALLING) {
    bob_amplitude_ = 0.0f;
    breathe_amplitude_ = 0.0f;
  }

  float h_speed = sqrtf(horizontal_velocity_.x * horizontal_velocity_.x +
                        horizontal_velocity_.z * horizontal_velocity_.z);
  Vector3 cam_fwd_h = Vector3Normalize({look_dir.x, 0.0f, look_dir.z});
  Vector3 cam_right =
      Vector3Normalize(Vector3CrossProduct(cam_fwd_h, {0.0f, 1.0f, 0.0f}));

  // Head bob
  update_head_bob(h_speed, cam_right, on_ground, dt);

  // Breathing
  update_breathing(h_speed, dt);

  // Landing squash + FOV
  update_landing_and_fov(dt);

  return {
      current_position_, {camera_.position.x, camera_.position.y},
      frame.jump,        false,
      frame.jump_held,
  };
}

// ── Ledge clamber: airborne + jump + ledge within reach ahead ────────
bool Player::advance_clamber(const MovementUpdate &frame, float dt) {
  if (!clambering_)
    return false;
  // Advance the mantle arc (smoothstepped), then camera-only frame.
  clamber_t_ += dt;
  float u = fminf(clamber_t_ / clamber_duration_, 1.0f);
  float s = u * u * (3.0f - 2.0f * u);
  current_position_.x =
      clamber_start_.x + (clamber_end_.x - clamber_start_.x) * s;
  current_position_.y =
      clamber_start_.y + (clamber_end_.y - clamber_start_.y) * s;
  current_position_.z =
      clamber_start_.z + (clamber_end_.z - clamber_start_.z) * s;
  if (u >= 1.0f) {
    clambering_ = false;
    vertical_velocity_ = 0.0f;
    jumps_remaining_ = max_jumps_;
  }
  // Suppress stale blink edges across the mantle.
  prev_blink_held_ = frame.blink_held;
  prev_recall_held_ = frame.recall_held;
  Vector3 look_dir = Vector3Subtract(camera_.target, camera_.position);
  camera_.position = current_position_;
  camera_.target = Vector3Add(camera_.position, look_dir);
  UpdateCameraPro(&camera_, Vector3{0},
                  Vector3{frame.camera.x * look_sensitivity_,
                          frame.camera.y * look_sensitivity_, 0.0f},
                  0.0f);
  return true;
}

// Vertical — jump buffer, coyote time, apex-hang gravity, jump-cut,
// ceiling/floor/landing-ledge resolution.
void Player::update_vertical(const MovementUpdate &frame, World *world,
                             float floor_y, bool on_ground, float dt) {
  if (frame.jump)
    jump_buffer_ = jump_buffer_frames_;
  else if (jump_buffer_ > 0)
    jump_buffer_--;

  if (on_ground)
    coyote_frames_ = coyote_time_frames_;
  else if (coyote_frames_ > 0)
    coyote_frames_--;

  if (jump_buffer_ > 0 && jumps_remaining_ > 0 &&
      (coyote_frames_ > 0 || jumps_remaining_ < max_jumps_)) {
    vertical_velocity_ = jump_force_;
    jumps_remaining_--;
    jump_buffer_ = 0;
    coyote_frames_ = 0;
    squash_offset_ = takeoff_kick_;
  } else if (on_ground) {
    vertical_velocity_ = 0.0f;
    current_position_.y = floor_y;
    jumps_remaining_ = max_jumps_;
  } else {
    float grav;
    if (vertical_velocity_ >= 0.0f) {
      float t = vertical_velocity_ / apex_hang_threshold_;
      if (t > 1.0f)
        t = 1.0f;
      grav =
          gravity_ * (apex_hang_reduction_ + (1.0f - apex_hang_reduction_) * t);
    } else {
      float t = -vertical_velocity_ / fall_blend_threshold_;
      if (t > 1.0f)
        t = 1.0f;
      grav = gravity_ * (1.0f + (fall_multiplier_ - 1.0f) * t);
    }
    vertical_velocity_ += grav * dt;
  }

  if (vertical_velocity_ > 0.0f && !frame.jump_held)
    vertical_velocity_ += jump_cut_gravity_ * dt;

  float new_y = current_position_.y + vertical_velocity_ * dt;
  if (vertical_velocity_ > 0.0f) {
    if (!world->is_ceiling_blocked(
            {current_position_.x, new_y, current_position_.z}))
      current_position_.y = new_y;
    else
      vertical_velocity_ = 0.0f;
  } else if (new_y < floor_y) {
    current_position_.y = floor_y;
    vertical_velocity_ = 0.0f;
    jumps_remaining_ = max_jumps_;
    jump_buffer_ = 0;
  } else {
    float landing_y = floor_y;
    for (float ddx : {-ground_probe_radius_, ground_probe_radius_}) {
      for (float ddz : {-ground_probe_radius_, ground_probe_radius_}) {
        float h = world->get_floor_height(current_position_.x + ddx,
                                          current_position_.z + ddz) +
                  eye_height_;
        if (h > landing_y && current_position_.y >= h)
          landing_y = h;
      }
    }
    if (new_y < landing_y) {
      current_position_.y = landing_y;
      vertical_velocity_ = 0.0f;
      jumps_remaining_ = max_jumps_;
      jump_buffer_ = 0;
      if (landing_y > floor_y)
        coyote_frames_ = coyote_time_frames_;
    } else {
      current_position_.y = new_y;
    }
  }

  if (prev_vertical_velocity_ < -2.0f && vertical_velocity_ >= 0.0f)
    squash_offset_ = -squash_amount_;
}

// Horizontal (blocked during preview and recall)
void Player::update_horizontal(const MovementUpdate &frame, World *world,
                               float floor_y, bool on_ground, float dt) {
  if (blink_state_ != BlinkState::PREVIEWING &&
      blink_state_ != BlinkState::RECALLING) {
    Vector3 dir = camera_relative_direction(frame.position);
    float input_len = sqrtf(dir.x * dir.x + dir.z * dir.z);
    if (input_len > 1.0f) {
      dir.x /= input_len;
      dir.z /= input_len;
    }

    bool has_input = input_len > 0.01f;
    bool sprint_just_pressed = frame.sprint && !prev_sprint_;
    prev_sprint_ = frame.sprint;
    if (sprint_mode_ == SprintMode::TOGGLE) {
      if (sprint_just_pressed && on_ground)
        sprint_active_ = !sprint_active_;
      if (!has_input || !on_ground)
        sprint_active_ = false;
    } else {
      sprint_active_ = frame.sprint && on_ground;
    }
    float speed = sprint_active_ ? sprint_speed_ : max_speed_;
    Vector3 target_vel = {dir.x * speed, 0.0f, dir.z * speed};
    float rate = (has_input ? accel_rate_ : decel_rate_) *
                 (on_ground ? 1.0f : air_control_);
    float t = rate * dt < 1.0f ? rate * dt : 1.0f;
    horizontal_velocity_.x += (target_vel.x - horizontal_velocity_.x) * t;
    horizontal_velocity_.z += (target_vel.z - horizontal_velocity_.z) * t;

    float ox = current_position_.x;
    float oz = current_position_.z;
    float ddx = horizontal_velocity_.x * dt;
    float ddz = horizontal_velocity_.z * dt;

    Vector3 full = {ox + ddx, current_position_.y, oz + ddz};
    if (world->position_is_acceptable(full)) {
      current_position_.x = full.x;
      current_position_.z = full.z;
    } else {
      // Blocked: try stepping/vaulting the ledge ahead before sliding.
      Vector3 pdir = dir;
      float plen = sqrtf(pdir.x * pdir.x + pdir.z * pdir.z);
      bool handled = false;
      if (plen > 0.01f) {
        pdir.x /= plen;
        pdir.z /= plen;
        Vector3 probe = {current_position_.x + pdir.x * ledge_probe_distance_,
                         current_position_.y,
                         current_position_.z + pdir.z * ledge_probe_distance_};
        float ledge_floor = world->get_floor_height(probe.x, probe.z);
        float rise = ledge_floor - (floor_y - eye_height_);
        Vector3 top = {probe.x, ledge_floor + eye_height_, probe.z};
        if (rise > 0.0f && world->position_is_acceptable(top)) {
          if (rise <= step_up_max_rise_ && on_ground) {
            // Step up small ledges while grounded.
            current_position_.x = probe.x;
            current_position_.z = probe.z;
            current_position_.y = top.y;
            vertical_velocity_ = 0.0f;
            handled = true;
          } else if (rise <= (on_ground ? clamber_max_rise_ground_
                                        : clamber_max_rise_air_)) {
            // Auto clamber: vault onto the ledge, no jump needed.
            // Extra reach while airborne — grab ledges as you fall past.
            clambering_ = true;
            clamber_t_ = 0.0f;
            clamber_start_ = current_position_;
            clamber_end_ = top;
            handled = true;
          }
        }
      }
      if (!handled) {
        Vector3 slide_x = {ox + ddx, current_position_.y, oz};
        if (world->position_is_acceptable(slide_x))
          current_position_.x = slide_x.x;
        else
          horizontal_velocity_.x = 0.0f;
        Vector3 slide_z = {current_position_.x, current_position_.y, oz + ddz};
        if (world->position_is_acceptable(slide_z))
          current_position_.z = slide_z.z;
        else
          horizontal_velocity_.z = 0.0f;
      }
    }
  } else {
    // Bleed off horizontal velocity while previewing
    horizontal_velocity_.x *= preview_velocity_damp_;
    horizontal_velocity_.z *= preview_velocity_damp_;
  }
}

// Recovery: if stuck inside geometry push upward
void Player::recover_from_overlap(World *world) {
  if (!world->position_is_acceptable(current_position_)) {
    for (int i = 0; i < recover_max_steps_; ++i) {
      current_position_.y += recover_step_height_;
      if (world->position_is_acceptable(current_position_)) {
        vertical_velocity_ = 0.0f;
        break;
      }
    }
  }
}

// Strafe tilt
void Player::update_strafe_tilt(Vector3 look_dir, float dt) {
  Vector3 cam_fwd_h = Vector3Normalize({look_dir.x, 0.0f, look_dir.z});
  Vector3 cam_right =
      Vector3Normalize(Vector3CrossProduct(cam_fwd_h, {0.0f, 1.0f, 0.0f}));
  float lateral_vel = horizontal_velocity_.x * cam_right.x +
                      horizontal_velocity_.z * cam_right.z;
  float tilt_target = lateral_vel / sprint_speed_ * tilt_max_angle_;
  if (tilt_target > tilt_max_angle_)
    tilt_target = tilt_max_angle_;
  if (tilt_target < -tilt_max_angle_)
    tilt_target = -tilt_max_angle_;
  tilt_current_ += (tilt_target - tilt_current_) * tilt_lerp_rate_ * dt;
  float tilt_rad = tilt_current_ * DEG2RAD;
  camera_.up.x = cam_right.x * sinf(tilt_rad);
  camera_.up.y = cosf(tilt_rad);
  camera_.up.z = cam_right.z * sinf(tilt_rad);
}

// Head bob
void Player::update_head_bob(float h_speed, Vector3 cam_right, bool on_ground,
                             float dt) {
  bob_timer_ += h_speed * bob_freq_ * dt;
  int step_phase = (int)(bob_timer_ / PI);
  if (step_phase != prev_step_phase_) {
    prev_step_phase_ = step_phase;
    if (on_ground && h_speed > 1.0f && blink_state_ != BlinkState::PREVIEWING)
      Audio::get().play(Sfx::Step, step_volume_);
  }
  float target_amp = h_speed > 0.5f ? 1.0f : 0.0f;
  bob_amplitude_ += (target_amp - bob_amplitude_) * bob_fade_rate_ * dt;
  float bob_scale =
      bob_amplitude_ * (sprint_active_ ? bob_sprint_scale_ : 1.0f);
  float bob_v = sinf(bob_timer_) * bob_height_ * bob_scale;
  float bob_l = sinf(bob_timer_ * 0.5f) * bob_lateral_ * bob_scale;
  camera_.position.y += bob_v;
  camera_.target.y += bob_v;
  camera_.position.x += cam_right.x * bob_l;
  camera_.position.z += cam_right.z * bob_l;
  camera_.target.x += cam_right.x * bob_l;
  camera_.target.z += cam_right.z * bob_l;
}

// Breathing
void Player::update_breathing(float h_speed, float dt) {
  breathe_timer_ += dt;
  float breathe_target = h_speed < 0.3f ? 1.0f : 0.0f;
  breathe_amplitude_ +=
      (breathe_target - breathe_amplitude_) * breathe_fade_rate_ * dt;
  float breathe = sinf(breathe_timer_ * breathe_freq_) * breathe_height_ *
                  breathe_amplitude_;
  camera_.position.y += breathe;
  camera_.target.y += breathe;
}

// Landing squash + FOV
void Player::update_landing_and_fov(float dt) {
  camera_.position.y += squash_offset_;
  camera_.target.y += squash_offset_;
  float target_fov = sprint_active_ ? base_fov_ + sprint_fov_bonus_ : base_fov_;
  camera_.fovy += (target_fov - camera_.fovy) * fov_lerp_rate_ * dt;
}

void Player::draw_hud(const Camera3D &view_camera) {
  // ── Blink vignette — thin white screen edges, subtle ────────────────
  if (blink_flash_ > 0.0f) {
    unsigned char a = (unsigned char)(blink_flash_ * blink_vignette_alpha_);
    Color c = {255, 255, 255, a};
    int w = GetScreenWidth(), h = GetScreenHeight();
    DrawRectangle(0, 0, w, blink_vignette_thickness_, c);
    DrawRectangle(0, h - blink_vignette_thickness_, w,
                  blink_vignette_thickness_, c);
    DrawRectangle(0, 0, blink_vignette_thickness_, h, c);
    DrawRectangle(w - blink_vignette_thickness_, 0, blink_vignette_thickness_,
                  h, c);
  }

  // ── Blue flash on anchor placement ──────────────────────────────────
  if (anchor_place_flash_ > 0.0f) {
    Color flash = BLUE;
    flash.a = (unsigned char)(anchor_place_flash_ * anchor_flash_alpha_);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), flash);
  }

  // ── Edge indicators for off-screen anchors during RECALLING ────────
  if (blink_state_ != BlinkState::RECALLING)
    return;

  float sw = (float)GetScreenWidth();
  float sh = (float)GetScreenHeight();
  Vector2 center = {sw * 0.5f, sh * 0.5f};
  float hw = center.x - edge_indicator_margin_;
  float hh = center.y - edge_indicator_margin_;

  Vector3 cam_fwd = Vector3Normalize(
      Vector3Subtract(view_camera.target, view_camera.position));

  for (int i = 0; i < anchor_list_.size(); i++) {
    Vector3 pos = anchor_list_.get(i);

    // Is the anchor in front of the camera?
    Vector3 to_anchor = Vector3Subtract(pos, view_camera.position);
    bool in_front = Vector3DotProduct(to_anchor, cam_fwd) > 0.0f;

    Vector2 screen_pos = GetWorldToScreen(pos, view_camera);
    bool on_screen = in_front && screen_pos.x >= 0 && screen_pos.x < sw &&
                     screen_pos.y >= 0 && screen_pos.y < sh;

    if (on_screen) {
      // 2D puck at the anchor's screen position — visible through walls
      bool sel = (i == selected_anchor_);
      Color pc = sel ? WHITE : BLUE;
      pc.a = sel ? puck_selected_alpha_ : puck_unselected_alpha_;
      DrawCircle((int)screen_pos.x, (int)screen_pos.y,
                 sel ? puck_selected_radius_ : puck_unselected_radius_, pc);
      DrawText(TextFormat("%d", i + 1), (int)screen_pos.x - 5,
               (int)screen_pos.y - 30, 16, WHITE);
      continue;
    }

    // Compute screen-space direction toward the anchor
    Vector2 dir;
    if (in_front) {
      dir = {screen_pos.x - center.x, screen_pos.y - center.y};
    } else {
      // Behind camera: flip so indicator points the right way
      dir = {center.x - screen_pos.x, center.y - screen_pos.y};
    }
    float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (len < 0.01f)
      continue;
    dir.x /= len;
    dir.y /= len;

    // Clamp to screen edge rectangle
    float edge_x, edge_y;
    if (fabsf(dir.x) * hh > fabsf(dir.y) * hw) {
      float scale = hw / fabsf(dir.x);
      edge_x = center.x + dir.x * scale;
      edge_y = center.y + dir.y * scale;
    } else {
      float scale = hh / fabsf(dir.y);
      edge_x = center.x + dir.x * scale;
      edge_y = center.y + dir.y * scale;
    }

    bool selected = (i == selected_anchor_);
    Color c = selected ? WHITE : BLUE;
    c.a = selected ? indicator_selected_alpha_ : indicator_unselected_alpha_;
    DrawCircle((int)edge_x, (int)edge_y,
               selected ? indicator_selected_radius_
                        : indicator_unselected_radius_,
               c);
    // Small arrow tip pointing inward
    DrawCircle((int)(edge_x - dir.x * indicator_arrow_offset_),
               (int)(edge_y - dir.y * indicator_arrow_offset_),
               selected ? indicator_arrow_selected_radius_
                        : indicator_arrow_unselected_radius_,
               c);
  }
}

void Player::draw() {
  if (blink_state_ == BlinkState::PREVIEWING) {
    Color c = BLUE;
    c.a = blink_marker_alpha_;
    // Render smoothed position; elevation type from unsmoothed (stays crisp)
    if (blink_target_elevated_) {
      DrawCylinder({blink_target_smooth_.x, blink_target_smooth_.y,
                    blink_target_smooth_.z},
                   blink_marker_radius_, 0.0f, blink_marker_height_, 8, c);
    } else {
      DrawSphere(blink_target_smooth_, blink_marker_radius_, c);
    }
  }

  if (blink_state_ == BlinkState::RECALLING) {
    for (int i = 0; i < anchor_list_.size(); i++) {
      Vector3 pos = anchor_list_.get(i);
      bool selected = (i == selected_anchor_);
      Color c = selected ? WHITE : BLUE;
      c.a = selected ? anchor_marker_selected_alpha_
                     : anchor_marker_unselected_alpha_;
      DrawSphere(pos,
                 selected ? anchor_marker_selected_radius_
                          : anchor_marker_unselected_radius_,
                 c);
      Color stem = c;
      stem.a = anchor_stem_alpha_;
      DrawCylinder({pos.x, pos.y - anchor_stem_height_, pos.z},
                   anchor_stem_radius_, anchor_stem_radius_,
                   anchor_stem_height_, 6, stem);
    }
  }
}
