#include "player.h"

// call base class constructor, similar to super().__init__() in python
Player::Player(Vector2 starting_position) : GameObject(starting_position) {}

void Player::draw() { DrawCircle(position.x, position.y, 50, BLUE); }
void Player::update(float dt) {
  const float base_speed = 500.0f;     // units per second
  const float speed = base_speed * dt; // scale by delta time

  if (IsKeyDown(KEY_W) && position.y - speed >= 50)
    position.y -= speed;
  if (IsKeyDown(KEY_S) && position.y + speed + 50 <= GetScreenHeight())
    position.y += speed;
  if (IsKeyDown(KEY_A) && position.x - speed >= 50)
    position.x -= speed;
  if (IsKeyDown(KEY_D) && position.x + speed + 50 <= GetScreenWidth())
    position.x += speed;
};
