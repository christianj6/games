#include "player.h"

Player::Player() : camera() {
  camera.position = (Vector3){0.0f, 2.0f, 4.0f}; // Camera position
  camera.target = (Vector3){0.0f, 2.0f, 0.0f};   // Camera looking at point
  camera.up =
      (Vector3){0.0f, 1.0f, 0.0f}; // Camera up vector (rotation towards target)
  camera.fovy = 60.0f;             // Camera field-of-view Y
  camera.projection = CAMERA_PERSPECTIVE; // Camera projection type

  int cameraMode = CAMERA_FIRST_PERSON;
}

void Player::handle_input() {
  float camera_sensitivity = 0.25f;
  UpdateCameraPro(
      &camera,
      (Vector3){
          (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) *
                  0.1f - // Move forward-backward
              (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) * 0.1f,
          (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) * 0.1f - // Move right-left
              (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) * 0.1f,
          0.0f // Move up-down
      },
      (Vector3){
          GetMouseDelta().x * camera_sensitivity, // Rotation: yaw
          GetMouseDelta().y * camera_sensitivity, // Rotation: pitch
          0.0f                                    // Rotation: roll
      },
      0.0f); // disable zoom
}

void Player::update(float dt) {
  // TODO
}

Camera Player::get_camera() { return camera; }
