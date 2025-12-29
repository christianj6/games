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

  const float speed = 4.5f;

  Vector3 updated_movement = {
      (right.x * movement.x + forward.x * movement.z) * speed * dt, 
      movement.y,  // preserve vertical movement calculated elsewhere
      (right.z * movement.x + forward.z * movement.z) * speed * dt};

  return updated_movement;
}

MovementUpdate Player::update(float dt, Blackboard &blackboard) {
  // get inputs and other update information
  MovementUpdate update = Actor::get_update(dt, blackboard);

  // handle jumping
  const float ground_height = 3.0f;
  if (update.jump && jumps_remaining_ > 0) {
    vertical_velocity_ = jump_force_;
    jumps_remaining_--;
  } else if (current_position.y <= ground_height) {
    vertical_velocity_ = 0.0f;
    current_position.y = ground_height;
    jumps_remaining_ = max_jumps_;
  } else {
    vertical_velocity_ += gravity_ * dt;
  }
  update.position.y = vertical_velocity_ * dt;

  // get candidate position
  update.position = adjust_movement_relative_to_camera(dt, update.position);
  Vector3 candidate_position = Vector3Add(current_position, update.position);

  // TODO: ask the world if it is okay to move into this position
  if (world->position_is_acceptable(candidate_position)) {
    current_position = candidate_position;
  }

  // Calculate how much the camera position is moving
  Vector3 old_camera_position = camera.position;
  Vector3 new_camera_position = current_position;
  Vector3 position_offset = Vector3Subtract(new_camera_position, old_camera_position);

  // Move both camera position AND target by the same offset
  // This maintains the look direction while moving (no orbiting effect)
  camera.position = new_camera_position;
  camera.target = Vector3Add(camera.target, position_offset);

  // Now apply mouse rotation on top of the maintained look direction
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
