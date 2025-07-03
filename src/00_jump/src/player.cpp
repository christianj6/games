#include "player.h"
#include "raylib.h"
#include <cmath>

Player::Player() {
  position_x = 100.0f;
  position_y = 100.0f;
  velocity = 0.0f;
}

void Player::update() {
  // Update player position and handle input
  if (IsKeyDown(KEY_SPACE) && position_y == 700.f) {
    // just use a simple check to make sure can only jump from the ground
    velocity = -10.0f;
  }

  velocity += 0.2f; // Gravity
  position_y += velocity;

  // Simple ground collision
  if (position_y > 700.0f) {
    position_y = 700.0f;
    velocity = 0.0f;
  }
}

void Player::draw() {
  // Draw the player as a simple rectangle
  DrawRectangle((int)position_x, (int)position_y, 50, 50, RED);
}

bool Player::is_hit(const Obstacle &obstacle) const {
  // Get obstacle position (we need to add a getter in Obstacle class)
  Vector2 player_center = {position_x + 25, position_y + 25};
  Vector2 obstacle_center = {obstacle.get_x(), obstacle.get_y()};

  // Calculate distance between centers
  float distance = sqrtf(powf(player_center.x - obstacle_center.x, 2) +
                         powf(player_center.y - obstacle_center.y, 2));

  // Player radius (half of 50) + obstacle radius (25)
  float min_distance = 25 + 25;

  return distance < min_distance;
}
