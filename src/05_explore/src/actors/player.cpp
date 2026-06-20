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

Vector3 add_vectors_xz_only(Vector3 v1, Vector3 v2) {
  // TODO: allow y axis movement and replace with raymath functions
  v1.x += v2.x;
  v1.z += v2.z;

  return v1;
}

Vector3 Player::adjust_movement_relative_to_camera(float dt, Vector3 movement) {
  Vector3 forward = {camera.target.x - camera.position.x,
                     0.0f,
                     camera.target.z - camera.position.z};
  forward = Vector3Normalize(forward);
  Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

  const float speed = 6.5f;

  Vector3 updated_movement = {
      (right.x * movement.x + forward.x * movement.z) * speed * dt,
      movement.y, // preserve vertical movement calculated elsewhere
      (right.z * movement.x + forward.z * movement.z) * speed * dt};

  return updated_movement;
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

  if (jump_buffer_ > 0 && jumps_remaining_ > 0 && coyote_frames_ > 0) {
    vertical_velocity_ = jump_force_;
    jumps_remaining_--;
    jump_buffer_ = 0;
    coyote_frames_ = 0;
  } else if (on_ground) {
    vertical_velocity_ = 0.0f;
    current_position.y = floor_y;
    jumps_remaining_ = max_jumps_;
  } else {
    float grav = vertical_velocity_ < 0.0f ? gravity_ * fall_multiplier_ : gravity_;
    if (fabsf(vertical_velocity_) < apex_hang_threshold_)
      grav *= apex_hang_reduction_;
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

  // horizontal: axis-separated for wall sliding
  update.position = adjust_movement_relative_to_camera(dt, update.position);
  float ox = current_position.x;
  float oz = current_position.z;

  Vector3 full = {ox + update.position.x, current_position.y,
                  oz + update.position.z};
  if (world->position_is_acceptable(full)) {
    current_position.x = full.x;
    current_position.z = full.z;
  } else {
    Vector3 slide_x = {ox + update.position.x, current_position.y, oz};
    if (world->position_is_acceptable(slide_x))
      current_position.x = slide_x.x;
    Vector3 slide_z = {current_position.x, current_position.y,
                       oz + update.position.z};
    if (world->position_is_acceptable(slide_z))
      current_position.z = slide_z.z;
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
