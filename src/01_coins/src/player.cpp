#include "player.h"
#include "raylib.h"

Player::Player() {
  x = GetScreenWidth() / 2.0f;
  y = GetScreenHeight() / 2.0f;
}

void Player::update() {
  const float speed = 5.0f;
  if (IsKeyDown(KEY_W))
    y -= speed;
  if (IsKeyDown(KEY_S))
    y += speed;
  if (IsKeyDown(KEY_A))
    x -= speed;
  if (IsKeyDown(KEY_D))
    x += speed;
}

void Player::draw() { DrawCircle(x, y, 50, BLUE); }
