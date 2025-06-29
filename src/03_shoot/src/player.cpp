#include "player.h"
#include "raylib.h"
#include "raymath.h"
#include "world.h"

/*
 * the player is just a camera with some collision
 * logic and the projectiles (spawned from the camera so we put them here)
 */
Player::Player() : camera() {
  // Start in the southwest corner, looking diagonally across map
  camera.position = (Vector3){-45.0f, 1.8f, 45.0f}; // SW corner
  camera.target = (Vector3){45.0f, 1.8f, -45.0f};   // Look toward NE corner
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;
}

Vector3 Player::try_move(Vector3 movement) const {
  if (!world)
    return movement;

  // Try full movement first
  Vector3 new_pos = {camera.position.x + movement.x, camera.position.y,
                     camera.position.z + movement.z};

  CollisionInfo collision = world->check_collision(new_pos);
  if (!collision.collision) {
    return movement;
  }

  // Try moving along X axis only
  Vector3 x_movement = {movement.x, 0, 0};
  new_pos = {camera.position.x + x_movement.x, camera.position.y,
             camera.position.z + x_movement.z};

  bool can_move_x = !world->check_collision(new_pos).collision;

  // Try moving along Z axis only
  Vector3 z_movement = {0, 0, movement.z};
  new_pos = {camera.position.x + z_movement.x, camera.position.y,
             camera.position.z + z_movement.z};

  bool can_move_z = !world->check_collision(new_pos).collision;

  // Handle different collision cases
  if (can_move_x && can_move_z) {
    // If both directions are free, use sliding as before
    Vector3 normal = collision.normal;
    float dot = movement.x * normal.x + movement.z * normal.z;
    return (Vector3){movement.x - normal.x * dot, 0,
                     movement.z - normal.z * dot};
  } else if (can_move_x) {
    return x_movement;
  } else if (can_move_z) {
    return z_movement;
  }

  // If no movement is possible, stop completely
  return (Vector3){0, 0, 0};
}

void Player::handle_input() {
  float camera_sensitivity = 0.095f; // Increased camera movement speed
  float speed = 0.08f;               // Reduced movement speed

  // Handle shooting
  if (has_weapon && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    // get a vector based on current camera direction to know where to shoot the
    // ball
    Vector3 direction =
        Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    // emplace back is more efficient than push bc no temporary object
    projectiles.emplace_back(camera.position, direction);
  }
  // TODO: make speed scale better with screen size
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

Vector3 Player::update(float dt) {
  // Check for weapon pickup
  if (!has_weapon && world) {
    // class stores a unique pointer and owns it; we just want to call the
    // functions so we get the raw pointer and call get_position
    const Weapon *weapon = world->get_weapon();
    // after weapon is picked up, get_weapon will return a nullptr, which is why
    // we need that check below however, because we effectively track weapon
    // pickup via the has_weapon flag, we should never end up here anyhow
    if (weapon) {
      float dist = Vector3Distance(camera.position, weapon->get_position());
      if (dist < 2.0f) {
        // by setting has_weapon before calling .reset on the weapon, we have
        // some additional behavior against unsafe behavior nevertheless this is
        // quite hacky
        has_weapon = true;
        world->consume_weapon();
      }
    }
  }

  // Update all projectiles
  for (auto it = projectiles.begin(); it != projectiles.end();) {
    // go through all the projectiles and update them
    it->update(dt);
    // std::find would be another useful function in this type of context
    if (!it->is_active()) {
      // if the active flag turned off, remove it from the vector to save
      // resources
      it = projectiles.erase(it);
    } else {
      ++it;
    }
  }
  return camera.position;
}

void Player::draw() {
  // there is nothing to draw for the player itself, drawing is really the
  // projectiles
  for (auto &projectile : projectiles) {
    projectile.draw();
  }
}

Camera Player::get_camera() { return camera; }
