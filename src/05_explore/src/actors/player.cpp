#include "player.h"
#include "actors/actor.h"
#include "systems/movement/controller.h"

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

MovementUpdate Player::update(float dt, Blackboard &blackboard) {
  // TODO: movement is made relative to camera direction
  MovementUpdate update = Actor::update(dt, blackboard);
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
