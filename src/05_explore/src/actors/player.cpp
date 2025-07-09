#include "player.h"
#include "actors/actor.h"

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

Vector3 Player::update(float dt, Blackboard &blackboard) {
  Actor::update(dt, blackboard);
  camera.position = current_position;

  return current_position;
}

void Player::draw() {
  // TODO
}
