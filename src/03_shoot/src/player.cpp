#include "player.h"
#include "raylib.h"
#include "raymath.h"

Player::Player() : camera() {
  camera.position = (Vector3){0.0f, 1.8f, 4.0f}; // More natural eye height
  camera.target = (Vector3){0.0f, 1.8f, 0.0f};   // Looking forward
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};       // Camera up vector
  camera.fovy = 60.0f; // Wider FOV for better visibility
  camera.projection = CAMERA_PERSPECTIVE;
}

Vector3 Player::try_move(Vector3 movement) const {
  if (!world)
    return movement;

  Vector3 new_pos = {camera.position.x + movement.x, camera.position.y,
                     camera.position.z + movement.z};

  // If movement causes collision, try moving along individual axes
  if (world->check_collision(new_pos)) {
    Vector3 try_x = {camera.position.x + movement.x, camera.position.y,
                     camera.position.z};
    Vector3 try_z = {camera.position.x, camera.position.y,
                     camera.position.z + movement.z};

    bool can_move_x = !world->check_collision(try_x);
    bool can_move_z = !world->check_collision(try_z);

    movement.x = can_move_x ? movement.x : 0.0f;
    movement.z = can_move_z ? movement.z : 0.0f;
  }

  return movement;
}

void Player::handle_input() {
  float camera_sensitivity = 0.095f; // Increased camera movement speed
  float speed = 0.15f;               // Reduced movement speed
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

  // Check for collisions and adjust movement
  movement = try_move(movement);

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

void Player::update(float dt) {
  // TODO
}

Camera Player::get_camera() { return camera; }
