#include "player.h"
#include "raylib.h"

Player::Player() {
  positionX = 100.0f;
  positionY = 100.0f;
  velocity = 0.0f;
}

void Player::update() {
  // Update player position and handle input
  if (IsKeyDown(KEY_SPACE)) {
    velocity = -5.0f;
  }

  velocity += 0.2f; // Gravity
  positionY += velocity;

  // Simple ground collision
  if (positionY > 700.0f) {
    positionY = 700.0f;
    velocity = 0.0f;
  }
}

void Player::draw() {
  // Draw the player as a simple rectangle
  DrawRectangle((int)positionX, (int)positionY, 50, 50, RED);
}
