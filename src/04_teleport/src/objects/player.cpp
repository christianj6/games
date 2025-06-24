#include "player.h"
#include "raymath.h"

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

  // Get forward and right vectors from camera
  Vector3 forward =
      Vector3Normalize(Vector3Subtract(camera.target, camera.position));
  Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

  // Reset movement vector
  Vector3 movement = {0};

  // WASD movement
  if (IsKeyDown(KEY_W))
    movement = Vector3Add(movement, forward);
  if (IsKeyDown(KEY_S))
    movement = Vector3Subtract(movement, forward);
  if (IsKeyDown(KEY_D))
    movement = Vector3Add(movement, right);
  if (IsKeyDown(KEY_A))
    movement = Vector3Subtract(movement, right);

  // Normalize and apply movement
  if (!Vector3Equals(movement, (Vector3){0})) {
    movement = Vector3Scale(Vector3Normalize(movement), MOVE_SPEED);
    camera.position = Vector3Add(camera.position, movement);
    camera.target = Vector3Add(camera.target, movement);
  }
}
