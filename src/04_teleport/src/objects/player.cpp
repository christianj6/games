#include "player.h"
#include "movement/controller.h"
#include "movement/input.h"
#include "raylib.h"
#include "raymath.h"
#include <memory>
#include <utility>

Player::Player()
    : movement_controller(std::make_unique<PlayerMovementController>(
          std::make_unique<KeyboardInputProvider>())) {
  // TODO: improve this player spawning
  // player is in corner of the map just above the ground
  camera.position = (Vector3){80.0f, 3.0f, 80.0f};
  camera.target = (Vector3){10.0f, 1.0f, 0.0f}; // Look forward along plane
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;
}

Player::Player(std::unique_ptr<IMovemementController> movement_controller)
    : movement_controller(std::move(movement_controller)) {
  // TODO: improve this player spawning
  // player is in corner of the map just above the ground
  camera.position = (Vector3){80.0f, 3.0f, 80.0f};
  camera.target = (Vector3){10.0f, 1.0f, 0.0f}; // Look forward along plane
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;
}

void Player::update(
    float dt,
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
        &vector_space_data) {
  Vector3 forward = {camera.target.x - camera.position.x,
                     camera.target.y - camera.position.y,
                     camera.target.z - camera.position.z};
  forward = Vector3Normalize(forward);
  Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
  Vector3 movement = movement_controller->update_movement(dt, forward, right);

  int block_x = static_cast<int>(camera.position.x + movement.x);
  int block_z = static_cast<int>(camera.position.z + movement.z);
  if (vector_space_data[2](block_x, block_z)) {
    // intended movement overlaps with a column; no movement
  } else {
    // no column collision; apply movement
    camera.position.x += movement.x;
    camera.position.z += movement.z;
    camera.target.x += movement.x;
    camera.target.z += movement.z;
  }

  move_camera();
}

void Player::move_camera() {
  float camera_sensitivity = 0.095f;
  UpdateCameraPro(&camera, (Vector3){0},
                  (Vector3){GetMouseDelta().x * camera_sensitivity,
                            GetMouseDelta().y * camera_sensitivity, 0.0f},
                  0.0f);
}

void Player::jump() {
  // TODO
}

void Player::blink() {
  // TODO: right-click spawns a ball like dishonored
  // TODO: ball cannot collide with obstacles
  // TODO: releasing blinks the player to the location of the ball with same
  // camera direction
}
