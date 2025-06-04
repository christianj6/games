#include "player.h"
#include "raylib.h"

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
