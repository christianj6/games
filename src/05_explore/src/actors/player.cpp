#include "player.h"
#include "actors/actor.h"
#include "raylib.h"
#include "raymath.h"
#include "systems/movement/controller.h"
#include <fmt/base.h>

Player::Player() {
  setup_camera();
  movement_controller = std::make_unique<UserMovementController>();
}

void Player::setup_camera() {
  // camera is slightly above the ground at the origin
  // TODO: better approach to synchronizing camera and obj position
  current_position = Vector3{20.0f, 3.0f, 20.0f};
  camera.position = Vector3{20.0f, 3.0f, 20.0f};
  camera.target = Vector3{50.0f, 3.0f, 50.0f};
  camera.up = Vector3{0.0f, 1.0f, 0.0f};
  camera.fovy = 60.0f;
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

MovementUpdate Player::update(float dt, Blackboard &blackboard) {
  World *world = blackboard.world;
  MovementUpdate update = Actor::get_update(dt, blackboard);

  const float eye_height = 2.0f;

  // vertical: snap to voxel-aware floor, apply gravity/jump
  float floor_y =
      world->get_floor_height(current_position.x, current_position.z) +
      eye_height;
  if (update.jump)
    jump_buffer_ = jump_buffer_frames_;
  else if (jump_buffer_ > 0)
    jump_buffer_--;

  bool on_ground = current_position.y <= floor_y && vertical_velocity_ <= 0.0f;
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
  } else if (on_ground) {
    vertical_velocity_ = 0.0f;
    current_position.y = floor_y;
    jumps_remaining_ = max_jumps_;
  } else {
    float grav;
    if (vertical_velocity_ >= 0.0f) {
      // rising: blend from apex-hang gravity (near top) to full gravity (fast rise)
      float t = vertical_velocity_ / apex_hang_threshold_;
      if (t > 1.0f) t = 1.0f;
      grav = gravity_ * (apex_hang_reduction_ + (1.0f - apex_hang_reduction_) * t);
    } else {
      // falling: blend from normal gravity (just past apex) to fall multiplier (fast fall)
      float t = -vertical_velocity_ / fall_blend_threshold_;
      if (t > 1.0f) t = 1.0f;
      grav = gravity_ * (1.0f + (fall_multiplier_ - 1.0f) * t);
    }
    vertical_velocity_ += grav * dt;
  }

  // jump cut: releasing jump early applies extra downward force for a shorter hop
  if (vertical_velocity_ > 0.0f && !update.jump_held)
    vertical_velocity_ += jump_cut_gravity_ * dt;

  float new_y = current_position.y + vertical_velocity_ * dt;
  if (vertical_velocity_ > 0.0f) {
    if (!world->is_ceiling_blocked({current_position.x, new_y, current_position.z})) {
      current_position.y = new_y;
    } else {
      vertical_velocity_ = 0.0f;
    }
  } else if (new_y < floor_y) {
    current_position.y = floor_y;
    vertical_velocity_ = 0.0f;
    jumps_remaining_ = max_jumps_;
    jump_buffer_ = 0;
  } else {
    // check AABB footprint corners to prevent clipping through raised surfaces
    float landing_y = floor_y;
    const float r = 0.3f;
    for (float dx : {-r, r}) {
      for (float dz : {-r, r}) {
        float h = world->get_floor_height(current_position.x + dx,
                                          current_position.z + dz) +
                  eye_height;
        if (h > landing_y && current_position.y >= h)
          landing_y = h;
      }
    }
    if (new_y < landing_y) {
      current_position.y = landing_y;
      vertical_velocity_ = 0.0f;
      jumps_remaining_ = max_jumps_;
      jump_buffer_ = 0;
    } else {
      current_position.y = new_y;
    }
  }

  // horizontal: accelerate velocity toward input target, axis-separated collision
  Vector3 dir = camera_relative_direction(update.position);
  float input_len = sqrtf(dir.x * dir.x + dir.z * dir.z);
  if (input_len > 1.0f) { dir.x /= input_len; dir.z /= input_len; }

  bool has_input = input_len > 0.01f;
  float speed = update.sprint ? sprint_speed_ : max_speed_;
  Vector3 target_vel = {dir.x * speed, 0.0f, dir.z * speed};
  float rate = has_input ? accel_rate_ : decel_rate_;
  float t = rate * dt < 1.0f ? rate * dt : 1.0f;
  horizontal_velocity_.x += (target_vel.x - horizontal_velocity_.x) * t;
  horizontal_velocity_.z += (target_vel.z - horizontal_velocity_.z) * t;

  float ox = current_position.x;
  float oz = current_position.z;
  float dx = horizontal_velocity_.x * dt;
  float dz = horizontal_velocity_.z * dt;

  Vector3 full = {ox + dx, current_position.y, oz + dz};
  if (world->position_is_acceptable(full)) {
    current_position.x = full.x;
    current_position.z = full.z;
  } else {
    Vector3 slide_x = {ox + dx, current_position.y, oz};
    if (world->position_is_acceptable(slide_x))
      current_position.x = slide_x.x;
    else
      horizontal_velocity_.x = 0.0f;
    Vector3 slide_z = {current_position.x, current_position.y, oz + dz};
    if (world->position_is_acceptable(slide_z))
      current_position.z = slide_z.z;
    else
      horizontal_velocity_.z = 0.0f;
  }

  // recovery: if stuck inside geometry push upward until free
  if (!world->position_is_acceptable(current_position)) {
    for (int i = 0; i < 20; ++i) {
      current_position.y += 0.1f;
      if (world->position_is_acceptable(current_position)) {
        vertical_velocity_ = 0.0f;
        break;
      }
    }
  }

  // sync camera position and target together to preserve look direction
  Vector3 position_offset =
      Vector3Subtract(current_position, camera.position);
  camera.position = current_position;
  camera.target = Vector3Add(camera.target, position_offset);

  float camera_sensitivity = 0.095f;
  UpdateCameraPro(&camera, Vector3{0},
                  Vector3{update.camera.x * camera_sensitivity,
                          update.camera.y * camera_sensitivity, 0.0f},
                  0.0f);
  return {
      current_position,
      {camera.position.x, camera.position.y},
      update.jump,
      false,
      update.jump_held,
  };
}

void Player::draw() {
  // TODO
}
