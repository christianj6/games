#include "player.h"

// call base class constructor, similar to super().__init__() in python
Player::Player(Vector2 starting_position) : GameObject(starting_position) {}

void Player::draw() { DrawCircle(position.x, position.y, 40, BLUE); }
void Player::update(float dt, bool allow_movement) {
  const float base_speed = 500.0f;     // units per second
  const float speed = base_speed * dt; // scale by delta time

  // Store original position before any movement
  Vector2 original_pos = position;

  // Try to move
  if (IsKeyDown(KEY_W))
    position.y -= speed;
  if (IsKeyDown(KEY_S))
    position.y += speed;
  if (IsKeyDown(KEY_A))
    position.x -= speed;
  if (IsKeyDown(KEY_D))
    position.x += speed;

  // If we're colliding with something, revert to original position
  if (!allow_movement) {
    position = original_pos;
  }
}
