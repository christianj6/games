#include "player.h"
#include "actors/actor.h"
#include "raylib.h"
#include "raymath.h"
#include "systems/movement/controller.h"
#include <cmath>
#include <fmt/base.h>

Player::Player() {
  setup_camera();
  movement_controller = std::make_unique<UserMovementController>();
}

void Player::setup_camera() {
  current_position = Vector3{20.0f, 3.0f, 20.0f};
  camera.position  = Vector3{20.0f, 3.0f, 20.0f};
  camera.target    = Vector3{50.0f, 3.0f, 50.0f};
  camera.up        = Vector3{0.0f,  1.0f, 0.0f};
  camera.fovy      = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;
}

Vector3 Player::camera_relative_direction(Vector3 input) {
  Vector3 forward = {camera.target.x - camera.position.x, 0.0f,
                     camera.target.z - camera.position.z};
  forward = Vector3Normalize(forward);
  Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
  return {right.x * input.x + forward.x * input.z, 0.0f,
          right.z * input.x + forward.z * input.z};
}

void Player::do_blink(Vector3 target, bool record) {
  if (record) jump_list_.push({current_position, target});
  Vector3 view_offset = Vector3Subtract(camera.target, camera.position);
  current_position = target;
  camera.position  = target;
  camera.target    = Vector3Add(target, view_offset);
  vertical_velocity_ = 0.0f;
}

void Player::handle_blink(const MovementUpdate &update, World *world) {
  // ── CHORD: LB+RB — place anchor, clear both button states ────────────
  if (update.place_anchor && !chord_active_) {
    anchor_list_.place(current_position);
    anchor_place_flash_ = 1.0f;
    chord_active_       = true;
    blink_state_        = BlinkState::IDLE; // cancel any in-progress blink
    blink_hold_frames_  = 0;
    recall_hold_frames_ = 0;
    return;
  }

  // Suppress all LB/RB actions until both buttons are fully released
  if (chord_active_) {
    prev_blink_held_    = update.blink_held;
    prev_recall_held_   = update.recall_held;
    blink_hold_frames_  = 0;
    recall_hold_frames_ = 0;
    if (!update.blink_held && !update.recall_held) {
      chord_active_ = false;
      blink_state_  = BlinkState::IDLE; // ensure clean state on exit
    }
    return;
  }

  // ── LB STATE MACHINE ─────────────────────────────────────────────────
  bool lb             = update.recall_held;
  bool lb_pressed     = lb  && !prev_recall_held_;
  bool lb_released    = !lb && prev_recall_held_;
  prev_recall_held_   = lb;

  if (lb_pressed)
    recall_hold_frames_ = 0;
  if (lb) {
    recall_hold_frames_++;
    if (recall_hold_frames_ >= recall_threshold_ &&
        blink_state_ == BlinkState::IDLE &&
        !anchor_list_.empty()) {
      blink_state_         = BlinkState::RECALLING;
      selected_anchor_     = -1;
      prev_stick_magnitude_ = 0.0f;
    }
  }
  if (lb_released) {
    if (blink_state_ == BlinkState::RECALLING) {
      if (selected_anchor_ >= 0 && selected_anchor_ < anchor_list_.size())
        do_blink(anchor_list_.get(selected_anchor_), false);
      blink_state_ = BlinkState::IDLE;
    } else if (recall_hold_frames_ < recall_threshold_) {
      // Quick tap: recall most recent blink position
      if (!jump_list_.empty())
        do_blink(jump_list_.pop().from, false);
    }
    recall_hold_frames_ = 0;
  }

  // ── RECALLING: flick right stick to select anchor ─────────────────
  if (blink_state_ == BlinkState::RECALLING) {
    float sx = fabsf(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X)) > 0.15f
                   ? GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X) : 0.0f;
    float sy = fabsf(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y)) > 0.15f
                   ? GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y) : 0.0f;
    float mag = sqrtf(sx * sx + sy * sy);

    // Rising edge — radial wheel keyed to screen-space position so the flick
    // direction matches exactly where the indicator (puck or edge dot) appears.
    if (mag > 0.5f && prev_stick_magnitude_ <= 0.5f) {
      float fx = sx / mag;
      float fy = -sy / mag; // stick Y inverted: push up = screen up = +fy
      float flick_angle = atan2f(fx, fy);

      Vector2 center = {GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f};
      Vector3 cam_fwd_n = Vector3Normalize(
          Vector3Subtract(camera.target, camera.position));

      int   best_i    = 0;
      float best_diff = 2.0f * PI;
      for (int i = 0; i < anchor_list_.size(); i++) {
        Vector3 pos = anchor_list_.get(i);

        // Project anchor to screen — same math used by the indicator
        Vector2 sp = GetWorldToScreen(pos, camera);
        bool in_front = Vector3DotProduct(
            Vector3Subtract(pos, camera.position), cam_fwd_n) > 0.0f;

        Vector2 dir = in_front ? Vector2{sp.x - center.x, sp.y - center.y}
                                : Vector2{center.x - sp.x, center.y - sp.y};
        float dlen = sqrtf(dir.x * dir.x + dir.y * dir.y);
        if (dlen < 0.01f) continue;

        // screen right = +x, screen up = -y → match stick convention
        float adx = dir.x / dlen;
        float ady = -dir.y / dlen; // invert Y: screen up maps to +fy
        float anchor_angle = atan2f(adx, ady);

        float diff = fabsf(flick_angle - anchor_angle);
        if (diff > PI) diff = 2.0f * PI - diff;
        if (diff < best_diff) { best_diff = diff; best_i = i; }
      }
      selected_anchor_ = best_i;
    }
    prev_stick_magnitude_ = mag;
  }

  // ── B: cancel PREVIEWING or RECALLING ────────────────────────────────
  if ((blink_state_ == BlinkState::PREVIEWING ||
       blink_state_ == BlinkState::RECALLING) &&
      IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
    blink_state_        = BlinkState::IDLE;
    blink_hold_frames_  = 0;
    recall_hold_frames_ = 0;
    selected_anchor_    = -1;
  }

  // ── RB STATE MACHINE — blocked during recall ──────────────────────────
  if (blink_state_ == BlinkState::RECALLING) return;

  bool held          = update.blink_held;
  bool just_released = !held && prev_blink_held_;
  bool just_pressed  =  held && !prev_blink_held_;
  prev_blink_held_   = held;

  if (just_pressed) {
    blink_state_       = BlinkState::HOLDING;
    blink_hold_frames_ = 0;
  }

  if (held) {
    if (blink_state_ == BlinkState::HOLDING) {
      blink_hold_frames_++;
      if (blink_hold_frames_ >= tap_threshold_)
        blink_state_ = BlinkState::PREVIEWING;
    }
    if (blink_state_ == BlinkState::PREVIEWING) {
      Vector3 dir = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
      blink_target_ = world->find_blink_target(current_position, dir, hold_blink_range_);
      float origin_floor = world->get_floor_height(current_position.x, current_position.z);
      float target_floor = world->get_floor_height(blink_target_.x, blink_target_.z);
      blink_target_elevated_ = (target_floor > origin_floor + 0.5f);
    }
  }

  if (just_released) {
    if (blink_state_ == BlinkState::HOLDING) {
      float h_spd = sqrtf(horizontal_velocity_.x * horizontal_velocity_.x +
                          horizontal_velocity_.z * horizontal_velocity_.z);
      Vector3 tap_dir;
      if (h_spd > 0.5f) {
        tap_dir = Vector3Normalize({horizontal_velocity_.x, 0.0f, horizontal_velocity_.z});
      } else {
        Vector3 fwd = Vector3Subtract(camera.target, camera.position);
        tap_dir = Vector3Normalize({fwd.x, 0.0f, fwd.z});
      }
      do_blink(world->find_blink_target_through(current_position, tap_dir, tap_blink_range_));
    } else if (blink_state_ == BlinkState::PREVIEWING) {
      do_blink(blink_target_);
    }
    blink_state_       = BlinkState::IDLE;
    blink_hold_frames_ = 0;
  }
}

MovementUpdate Player::update(float dt, Blackboard &blackboard) {
  World *world = blackboard.world;
  MovementUpdate update = Actor::get_update(dt, blackboard);

  const float eye_height = 2.0f;

  // Time dilation: hold-blink scales world/actor dt via blackboard
  blackboard.time_scale = update.blink_held ? blink_time_scale_ : 1.0f;

  // Vertical
  float floor_y = world->get_floor_height(current_position.x, current_position.z)
                  + eye_height;
  bool on_ground = current_position.y <= floor_y && vertical_velocity_ <= 0.0f;

  if (blink_state_ == BlinkState::PREVIEWING ||
      blink_state_ == BlinkState::RECALLING) {
    // Suspend the player in place during preview and recall
    vertical_velocity_   = 0.0f;
    current_position.y   = std::max(current_position.y, floor_y);
    on_ground            = true;
  } else {
    if (update.jump)
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
      jump_buffer_   = 0;
      coyote_frames_ = 0;
      squash_offset_ = takeoff_kick_;
    } else if (on_ground) {
      vertical_velocity_ = 0.0f;
      current_position.y = floor_y;
      jumps_remaining_   = max_jumps_;
    } else {
      float grav;
      if (vertical_velocity_ >= 0.0f) {
        float t = vertical_velocity_ / apex_hang_threshold_;
        if (t > 1.0f) t = 1.0f;
        grav = gravity_ * (apex_hang_reduction_ + (1.0f - apex_hang_reduction_) * t);
      } else {
        float t = -vertical_velocity_ / fall_blend_threshold_;
        if (t > 1.0f) t = 1.0f;
        grav = gravity_ * (1.0f + (fall_multiplier_ - 1.0f) * t);
      }
      vertical_velocity_ += grav * dt;
    }

    if (vertical_velocity_ > 0.0f && !update.jump_held)
      vertical_velocity_ += jump_cut_gravity_ * dt;

    float new_y = current_position.y + vertical_velocity_ * dt;
    if (vertical_velocity_ > 0.0f) {
      if (!world->is_ceiling_blocked({current_position.x, new_y, current_position.z}))
        current_position.y = new_y;
      else
        vertical_velocity_ = 0.0f;
    } else if (new_y < floor_y) {
      current_position.y = floor_y;
      vertical_velocity_ = 0.0f;
      jumps_remaining_   = max_jumps_;
      jump_buffer_       = 0;
    } else {
      float landing_y = floor_y;
      const float r = 0.3f;
      for (float ddx : {-r, r}) {
        for (float ddz : {-r, r}) {
          float h = world->get_floor_height(current_position.x + ddx,
                                            current_position.z + ddz) + eye_height;
          if (h > landing_y && current_position.y >= h)
            landing_y = h;
        }
      }
      if (new_y < landing_y) {
        current_position.y = landing_y;
        vertical_velocity_ = 0.0f;
        jumps_remaining_   = max_jumps_;
        jump_buffer_       = 0;
        if (landing_y > floor_y)
          coyote_frames_ = coyote_time_frames_;
      } else {
        current_position.y = new_y;
      }
    }

    if (prev_vertical_velocity_ < -2.0f && vertical_velocity_ >= 0.0f)
      squash_offset_ = -squash_amount_;
  }

  prev_vertical_velocity_ = vertical_velocity_;
  squash_offset_ += (0.0f - squash_offset_) * squash_spring_rate_ * dt;

  // Horizontal (blocked during preview and recall)
  if (blink_state_ != BlinkState::PREVIEWING &&
      blink_state_ != BlinkState::RECALLING) {
    Vector3 dir = camera_relative_direction(update.position);
    float input_len = sqrtf(dir.x * dir.x + dir.z * dir.z);
    if (input_len > 1.0f) { dir.x /= input_len; dir.z /= input_len; }

    bool has_input = input_len > 0.01f;
    bool sprint_just_pressed = update.sprint && !prev_sprint_;
    prev_sprint_ = update.sprint;
    if (sprint_mode_ == SprintMode::Toggle) {
      if (sprint_just_pressed && on_ground)
        sprint_active_ = !sprint_active_;
      if (!has_input || !on_ground)
        sprint_active_ = false;
    } else {
      sprint_active_ = update.sprint && on_ground;
    }
    float speed = sprint_active_ ? sprint_speed_ : max_speed_;
    Vector3 target_vel = {dir.x * speed, 0.0f, dir.z * speed};
    float rate = (has_input ? accel_rate_ : decel_rate_) *
                 (on_ground ? 1.0f : air_control_);
    float t = rate * dt < 1.0f ? rate * dt : 1.0f;
    horizontal_velocity_.x += (target_vel.x - horizontal_velocity_.x) * t;
    horizontal_velocity_.z += (target_vel.z - horizontal_velocity_.z) * t;

    float ox = current_position.x;
    float oz = current_position.z;
    float ddx = horizontal_velocity_.x * dt;
    float ddz = horizontal_velocity_.z * dt;

    Vector3 full = {ox + ddx, current_position.y, oz + ddz};
    if (world->position_is_acceptable(full)) {
      current_position.x = full.x;
      current_position.z = full.z;
    } else {
      Vector3 slide_x = {ox + ddx, current_position.y, oz};
      if (world->position_is_acceptable(slide_x))
        current_position.x = slide_x.x;
      else
        horizontal_velocity_.x = 0.0f;
      Vector3 slide_z = {current_position.x, current_position.y, oz + ddz};
      if (world->position_is_acceptable(slide_z))
        current_position.z = slide_z.z;
      else
        horizontal_velocity_.z = 0.0f;
    }
  } else {
    // Bleed off horizontal velocity while previewing
    horizontal_velocity_.x *= 0.8f;
    horizontal_velocity_.z *= 0.8f;
  }

  // Recovery: if stuck inside geometry push upward
  if (!world->position_is_acceptable(current_position)) {
    for (int i = 0; i < 20; ++i) {
      current_position.y += 0.1f;
      if (world->position_is_acceptable(current_position)) {
        vertical_velocity_ = 0.0f;
        break;
      }
    }
  }

  // Camera sync
  Vector3 look_dir = Vector3Subtract(camera.target, camera.position);
  camera.position = current_position;
  camera.target   = Vector3Add(camera.position, look_dir);

  // Strafe tilt
  Vector3 cam_fwd_h = Vector3Normalize({look_dir.x, 0.0f, look_dir.z});
  Vector3 cam_right = Vector3Normalize(Vector3CrossProduct(cam_fwd_h, {0.0f, 1.0f, 0.0f}));
  float lateral_vel = horizontal_velocity_.x * cam_right.x +
                      horizontal_velocity_.z * cam_right.z;
  float tilt_target = lateral_vel / sprint_speed_ * tilt_max_angle_;
  if (tilt_target >  tilt_max_angle_) tilt_target =  tilt_max_angle_;
  if (tilt_target < -tilt_max_angle_) tilt_target = -tilt_max_angle_;
  tilt_current_ += (tilt_target - tilt_current_) * tilt_lerp_rate_ * dt;
  float tilt_rad = tilt_current_ * DEG2RAD;
  camera.up.x = cam_right.x * sinf(tilt_rad);
  camera.up.y = cosf(tilt_rad);
  camera.up.z = cam_right.z * sinf(tilt_rad);

  float camera_sensitivity = 0.095f;
  bool suppress_look = (blink_state_ == BlinkState::RECALLING);
  UpdateCameraPro(&camera, Vector3{0},
                  suppress_look ? Vector3{0, 0, 0}
                                : Vector3{update.camera.x * camera_sensitivity,
                                          update.camera.y * camera_sensitivity, 0.0f},
                  0.0f);

  // Blink state machine (uses camera direction after mouse look)
  handle_blink(update, world);

  // Decay anchor placement flash
  if (anchor_place_flash_ > 0.0f)
    anchor_place_flash_ = std::max(0.0f, anchor_place_flash_ - dt * 4.0f);

  // Kill bob and breathe during recall — camera must be still for selection
  if (blink_state_ == BlinkState::RECALLING) {
    bob_amplitude_    = 0.0f;
    breathe_amplitude_ = 0.0f;
  }

  // Head bob
  float h_speed = sqrtf(horizontal_velocity_.x * horizontal_velocity_.x +
                        horizontal_velocity_.z * horizontal_velocity_.z);
  bob_timer_ += h_speed * bob_freq_ * dt;
  float target_amp = h_speed > 0.5f ? 1.0f : 0.0f;
  bob_amplitude_ += (target_amp - bob_amplitude_) * bob_fade_rate_ * dt;
  float bob_scale = bob_amplitude_ * (sprint_active_ ? bob_sprint_scale_ : 1.0f);
  float bob_v = sinf(bob_timer_) * bob_height_ * bob_scale;
  float bob_l = sinf(bob_timer_ * 0.5f) * bob_lateral_ * bob_scale;
  camera.position.y += bob_v;
  camera.target.y   += bob_v;
  camera.position.x += cam_right.x * bob_l;
  camera.position.z += cam_right.z * bob_l;
  camera.target.x   += cam_right.x * bob_l;
  camera.target.z   += cam_right.z * bob_l;

  // Breathing
  breathe_timer_ += dt;
  float breathe_target = h_speed < 0.3f ? 1.0f : 0.0f;
  breathe_amplitude_ += (breathe_target - breathe_amplitude_) * breathe_fade_rate_ * dt;
  float breathe = sinf(breathe_timer_ * breathe_freq_) * breathe_height_ * breathe_amplitude_;
  camera.position.y += breathe;
  camera.target.y   += breathe;

  // Landing squash + FOV
  camera.position.y += squash_offset_;
  camera.target.y   += squash_offset_;
  float target_fov = sprint_active_ ? base_fov_ + sprint_fov_bonus_ : base_fov_;
  camera.fovy += (target_fov - camera.fovy) * fov_lerp_rate_ * dt;

  return {
      current_position,
      {camera.position.x, camera.position.y},
      update.jump,
      false,
      update.jump_held,
  };
}

void Player::draw_hud(Camera3D camera) {
  // ── Blue flash on anchor placement ──────────────────────────────────
  if (anchor_place_flash_ > 0.0f) {
    Color flash = BLUE;
    flash.a = (unsigned char)(anchor_place_flash_ * 100);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), flash);
  }

  // ── Edge indicators for off-screen anchors during RECALLING ────────
  if (blink_state_ != BlinkState::RECALLING) return;

  float sw = (float)GetScreenWidth();
  float sh = (float)GetScreenHeight();
  Vector2 center = {sw * 0.5f, sh * 0.5f};
  const float margin = 45.0f;
  float hw = center.x - margin;
  float hh = center.y - margin;

  Vector3 cam_fwd = Vector3Normalize(
      Vector3Subtract(camera.target, camera.position));

  for (int i = 0; i < anchor_list_.size(); i++) {
    Vector3 pos = anchor_list_.get(i);

    // Is the anchor in front of the camera?
    Vector3 to_anchor = Vector3Subtract(pos, camera.position);
    bool in_front = Vector3DotProduct(to_anchor, cam_fwd) > 0.0f;

    Vector2 screen_pos = GetWorldToScreen(pos, camera);
    bool on_screen = in_front &&
                     screen_pos.x >= 0 && screen_pos.x < sw &&
                     screen_pos.y >= 0 && screen_pos.y < sh;

    if (on_screen) {
      // 2D puck at the anchor's screen position — visible through walls
      bool sel = (i == selected_anchor_);
      Color pc = sel ? WHITE : BLUE; pc.a = sel ? 220 : 160;
      DrawCircle((int)screen_pos.x, (int)screen_pos.y, sel ? 9.0f : 6.0f, pc);
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
    if (len < 0.01f) continue;
    dir.x /= len; dir.y /= len;

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
    c.a = selected ? 230 : 180;
    DrawCircle((int)edge_x, (int)edge_y, selected ? 10.0f : 7.0f, c);
    // Small arrow tip pointing inward
    DrawCircle((int)(edge_x - dir.x * 14), (int)(edge_y - dir.y * 14),
               selected ? 5.0f : 3.5f, c);
  }
}

void Player::draw() {
  if (blink_state_ == BlinkState::PREVIEWING) {
    Color c = BLUE; c.a = 120;
    if (blink_target_elevated_) {
      DrawCylinder({blink_target_.x, blink_target_.y, blink_target_.z},
                   0.5f, 0.0f, 1.2f, 8, c);
    } else {
      DrawSphere(blink_target_, 0.5f, c);
    }
  }

  if (blink_state_ == BlinkState::RECALLING) {
    for (int i = 0; i < anchor_list_.size(); i++) {
      Vector3 pos      = anchor_list_.get(i);
      bool    selected = (i == selected_anchor_);
      Color   c        = selected ? WHITE : BLUE;
      c.a              = selected ? 230 : 160;
      DrawSphere(pos, selected ? 0.65f : 0.4f, c);
      Color stem = c; stem.a = 70;
      DrawCylinder({pos.x, pos.y - 1.5f, pos.z}, 0.04f, 0.04f, 1.5f, 6, stem);
    }
  }
}
