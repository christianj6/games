#include "player.h"
#include "raylib.h"

Player::Player() {
  float x = GetScreenWidth() / 2.0f;
  float y = GetScreenHeight() / 2.0f;
  position = {x, y};
}

void Player::update() {
  const float speed = 5.0f;
  if (IsKeyDown(KEY_W))
    position.y -= speed;
  if (IsKeyDown(KEY_S))
    position.y += speed;
  if (IsKeyDown(KEY_A))
    position.x -= speed;
  if (IsKeyDown(KEY_D))
    position.x += speed;
}

void Player::draw() { DrawCircle(position.x, position.y, 50, BLUE); }
Vector2 Player::get_position() { return position; }
