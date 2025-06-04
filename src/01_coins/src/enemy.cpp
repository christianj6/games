#include "enemy.h"
#include "raylib.h"
#include <math.h>

Enemy::Enemy() {
  position.x = GetScreenWidth() - 50.0f; // Offset by radius to be fully visible
  position.y = 50.0f;                    // Offset from top by radius
}

void Enemy::update(Vector2 player_position) {
  const float speed = 3.0f; // Slower than player for fairness

  // Calculate direction to player
  Vector2 direction = {player_position.x - position.x,
                       player_position.y - position.y};

  // Normalize the direction
  float length = sqrt(direction.x * direction.x + direction.y * direction.y);
  if (length > 0) {
    direction.x /= length;
    direction.y /= length;

    // Move towards player
    position.x += direction.x * speed;
    position.y += direction.y * speed;
  }
}

void Enemy::draw() { DrawCircle(position.x, position.y, 25, RED); }
