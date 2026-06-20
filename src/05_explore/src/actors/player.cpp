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
                     camera.target.y - camera.position.y,
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

  if (jump_buffer_ > 0 && jumps_remaining_ > 0) {
    vertical_velocity_ = jump_force_;
    jumps_remaining_--;
    jump_buffer_ = 0;
  } else if (current_position.y <= floor_y && vertical_velocity_ <= 0.0f) {
    vertical_velocity_ = 0.0f;
    current_position.y = floor_y;
    jumps_remaining_ = max_jumps_;
  } else {
    vertical_velocity_ += gravity_ * dt;
  }
  Vector3 after_vertical = {current_position.x,
                             current_position.y + vertical_velocity_ * dt,
                             current_position.z};
  if (world->position_is_acceptable(after_vertical)) {
    current_position.y = after_vertical.y;
  } else if (vertical_velocity_ > 0.0f) {
    vertical_velocity_ = 0.0f; // hit ceiling
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
  };
}

void Player::draw() {
  // TODO
}
