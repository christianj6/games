#include "player.h"
#include "raymath.h"

Player::Player() {
  // TODO: improve this player spawning
  // player is in corner of the map just above the ground
  camera.position = (Vector3){80.0f, 3.0f, 80.0f};
  camera.target = (Vector3){10.0f, 1.0f, 0.0f}; // Look forward along plane
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;
}

void Player::update(float dt) { handle_input(); }

void Player::jump() {
  // TODO
}

void Player::blink() {
  // TODO
}

void Player::handle_input() {
  float camera_sensitivity = 0.095f; // Increased camera movement speed
  float speed = 0.08f;               // Reduced movement speed

  // Handle shooting
  // Get forward vector (normalized direction vector from position to target)
  Vector3 forward = {camera.target.x - camera.position.x,
                     camera.target.y - camera.position.y,
                     camera.target.z - camera.position.z};
  forward = Vector3Normalize(forward);

  // Calculate right vector (cross product of forward and up)
  Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

  // Initialize movement vector
  Vector3 movement = {0};

  // Forward/Backward movement along forward vector
  if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
    movement.x += forward.x * speed;
    movement.z += forward.z * speed;
  }
  if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
    movement.x -= forward.x * speed;
    movement.z -= forward.z * speed;
  }

  // Left/Right movement along right vector
  if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
    movement.x += right.x * speed;
    movement.z += right.z * speed;
  }
  if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
    movement.x -= right.x * speed;
    movement.z -= right.z * speed;
  }

  /*// Check for collisions and adjust movement*/
  /*movement = try_move(movement);*/

  // Apply the movement
  camera.position.x += movement.x;
  camera.position.z += movement.z;
  camera.target.x += movement.x;
  camera.target.z += movement.z;

  // Handle rotation
  UpdateCameraPro(&camera, (Vector3){0}, // Movement handled above
                  (Vector3){GetMouseDelta().x * camera_sensitivity,
                            GetMouseDelta().y * camera_sensitivity, 0.0f},
                  0.0f);
}
