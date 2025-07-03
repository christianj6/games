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
  setup_camera();
}

Player::Player(std::unique_ptr<IMovemementController> movement_controller)
    : movement_controller(std::move(movement_controller)) {
  setup_camera();
}

void Player::reset_position() {
  camera.position = Vector3{80.0f, 3.0f, 80.0f};
  camera.target = Vector3{10.0f, 1.0f, 0.0f}; // Look forward along plane
}

void Player::setup_camera() {
  camera.position = Vector3{80.0f, 3.0f, 80.0f};
  camera.target = Vector3{10.0f, 1.0f, 0.0f}; // Look forward along plane
  camera.up = Vector3{0.0f, 1.0f, 0.0f};
  camera.fovy = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;
}

// TODO: extract collision detection into abstraction
PlayerAction Player::update(
    float dt,
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
        &vector_space_data) {
  // Handle jumping
  jump();
  blink();

  if (!is_blinking) {
    // Apply gravity and update vertical position
    vertical_velocity += GRAVITY * dt;
    camera.position.y += vertical_velocity * dt;
    camera.target.y += vertical_velocity * dt;

    // Ground check using voxel space
    int current_x = static_cast<int>(camera.position.x);
    int current_z = static_cast<int>(camera.position.z);
    int current_y = static_cast<int>(camera.position.y);
    float ground_height = 3.0f; // Default ground height

    // Check for blocks below us
    if (current_x >= 0 && current_z >= 0 &&
        current_x < vector_space_data[0].rows() &&
        current_z < vector_space_data[0].cols()) {

      // Check each layer from current position down
      for (int y = current_y; y >= 0 && y < vector_space_data.size(); --y) {
        if (vector_space_data[y](current_x, current_z)) {
          ground_height =
              y + 3.0f; // Convert voxel Y to world Y + player height
          break;
        }
      }
    }

    if (camera.position.y <= ground_height) {
      camera.position.y = ground_height;
      vertical_velocity = 0.0f;
      jumps_remaining = MAX_JUMPS;
    }

    Vector3 forward = {camera.target.x - camera.position.x,
                       camera.target.y - camera.position.y,
                       camera.target.z - camera.position.z};
    forward = Vector3Normalize(forward);
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
    Vector3 movement = movement_controller->update_movement(dt, forward, right);

    int block_x = static_cast<int>(camera.position.x + movement.x);
    int block_z = static_cast<int>(camera.position.z + movement.z);

    // Check if movement would collide with a block
    if (block_x >= 0 && block_z >= 0 && block_x < vector_space_data[1].rows() &&
        block_z < vector_space_data[1].cols() &&
        vector_space_data[1](block_x, block_z)) {
      // If we're above block height, allow movement
      if (camera.position.y >= 5.0f) {
        camera.position.x += movement.x;
        camera.position.z += movement.z;
        camera.target.x += movement.x;
        camera.target.z += movement.z;
      }
      // Otherwise block movement
    } else {
      // No block collision; apply movement
      camera.position.x += movement.x;
      camera.position.z += movement.z;
      camera.target.x += movement.x;
      camera.target.z += movement.z;
    }
  }
  move_camera();

  return {attack(), is_blinking};
}

void Player::draw(
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
        &vector_space_data) {
  if (is_blinking) {
    Vector3 direction = Vector3Subtract(camera.target, camera.position);
    direction = Vector3Normalize(direction); // Normalize the direction vector

    // Ray step size
    const float max_distance = 20.0f; // Increased for better visibility
    const float step = 0.1f;
    bool found_collision = false;

    // Cast ray forward until we hit something or reach max distance
    for (float dist = 0; dist <= max_distance && !found_collision;
         dist += step) {
      blink_target = Vector3Add(camera.position, Vector3Scale(direction, dist));
      int check_x = static_cast<int>(blink_target.x);
      int check_z = static_cast<int>(blink_target.z);
      int check_y = static_cast<int>(blink_target.y);

      // Check if we hit a block
      if (vector_space_data[check_y](check_x, check_z)) {
        found_collision = true;
        // Step back slightly from collision
        blink_target = Vector3Add(
            camera.position,
            Vector3Scale(direction,
                         dist - step * 6)); // move back 6*step so ball does not
                                            // overlap w obstacles
        break;
      }
    }

    Color sphere_color = BLUE;
    sphere_color.a = 100;
    DrawSphere(blink_target, 0.75f, sphere_color);
  }
}

void Player::move_camera() {
  Vector2 camera_update = movement_controller->update_camera();
  float camera_sensitivity = 0.095f;
  // float camera_sensitivity = 1.0f; // wsl
  UpdateCameraPro(&camera, Vector3{0},
                  Vector3{camera_update.x * camera_sensitivity,
                          camera_update.y * camera_sensitivity, 0.0f},
                  0.0f);
}

void Player::jump() {
  if (jumps_remaining > 0 && movement_controller->get_input_jump()) {
    vertical_velocity = JUMP_FORCE;
    jumps_remaining--;
  }
}

void Player::blink() {
  if (is_blinking) {
    if (!movement_controller->get_blink_input()) {
      // Calculate the offset between current target and position
      Vector3 view_offset = Vector3Subtract(camera.target, camera.position);

      // Update position to blink target
      camera.position = blink_target;

      // Update target to maintain the same view direction
      camera.target = Vector3Add(camera.position, view_offset);
    }
  }
  is_blinking = movement_controller->get_blink_input();
}

bool Player::attack() { return movement_controller->get_attack_input(); }
