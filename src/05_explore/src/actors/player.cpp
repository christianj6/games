#include "player.h"
#include "actors/actor.h"
#include "raylib.h"
#include "systems/movement/controller.h"
#include "raymath.h"

Player::Player() {
  setup_camera();
  movement_controller = std::make_unique<UserMovementController>();
}

void Player::setup_camera() {
  // camera is slightly above the ground at the origin
  // TODO: better approach to synchronizing camera and obj position
  current_position = Vector3{0.0f, 3.0f, 0.0f};
  camera.position = Vector3{0.0f, 3.0f, 0.0f};
  // looking across the play area
  camera.target = Vector3{80.0f, 3.0f, 80.0f};
  // camera.up = Vector3{0.0f, 1.0f, 0.0f};
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

  Vector3 updated_movement = {(right.x * movement.x +
                       forward.x * movement.z) *
                          speed * dt,
                      0.0f,
                      (right.z * movement.x +
                       forward.z * movement.z) *
                          speed * dt};
  
  return updated_movement;
}

MovementUpdate Player::update(float dt, Blackboard &blackboard) {
  MovementUpdate update = Actor::get_update(dt, blackboard);
  update.position = adjust_movement_relative_to_camera(dt, update.position);
  Vector3 candidate_position = add_vectors_xz_only(current_position, update.position);

  if (world->position_is_acceptable(candidate_position)) {
    current_position = candidate_position;
  }

  camera.position = current_position;

  float camera_sensitivity = 0.095f;
  UpdateCameraPro(&camera, Vector3{0},
                  Vector3{update.camera.x * camera_sensitivity,
                          update.camera.y * camera_sensitivity, 0.0f},
                  0.0f);
  return {
      current_position,
      {camera.position.x, camera.position.y},
      false,
      false,
  };
}

void Player::draw() {
  // TODO
}
