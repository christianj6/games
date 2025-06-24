#include "player.h"

Player::Player() {
  camera.position = (Vector3){-125.0f, 125.0f, -125.0f};
  camera.target = (Vector3){0.0f, 0.0f, 0.0f};
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 45.0f;
  camera.projection = CAMERA_PERSPECTIVE;
}

void Player::update(float dt) { handle_input(); }

void Player::handle_input() {
  // camera movement
  float camera_sensitivity = 0.095f;
  UpdateCameraPro(&camera, (Vector3){0},
                  (Vector3){GetMouseDelta().x * camera_sensitivity,
                            GetMouseDelta().y * camera_sensitivity, 0.0f},
                  0.0f);
  // TODO: player movement
}
